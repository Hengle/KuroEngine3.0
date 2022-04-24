#include "Importer.h"
#include"KuroFunc.h"
#include<Windows.h>
#include"Model.h"
#include"D3D12App.h"

void Importer::ErrorMessage(const std::string& FuncName, const bool& Fail, const std::string& Comment)
{
	KuroFunc::ErrorMessage(Fail, "Importer", FuncName, Comment);
}

bool Importer::LoadData(FILE* Fp, void* Data, const size_t& Size, const int& ElementNum)
{
	return 1 <= fread(Data, Size, ElementNum, Fp);
}

bool Importer::SaveData(FILE* Fp, const void* Data, const size_t& Size, const int& ElementNum)
{
	return 1 <= fwrite(Data, Size, ElementNum, Fp);
}

void Importer::FbxDeviceDestroy()
{
	if (fbxImporter)fbxImporter->Destroy();
	if (ioSettings)ioSettings->Destroy();
	if (fbxManager)fbxManager->Destroy();
}

void Importer::TraceFbxMesh(FbxNode* Node, std::vector<FbxMesh*>* Mesh)
{
	int childCount = Node->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		FbxNode* child = Node->GetChild(i);	//子ノードを取得
		int meshCount = child->GetNodeAttributeCount();
		for (int num = 0; num < meshCount; num++)
		{
			FbxNodeAttribute* info = child->GetNodeAttributeByIndex(num);
			FbxNodeAttribute::EType type = info->GetAttributeType();

			if (type == FbxNodeAttribute::EType::eMesh)
			{
				Mesh->emplace_back((FbxMesh*)info);
			}
		}
		TraceFbxMesh(child, Mesh);
	}
}

XMMATRIX Importer::FbxMatrixConvert(const FbxMatrix& Mat)
{
	XMMATRIX result;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result.r[i].m128_f32[j] = Mat[i][j];
		}
	}
	return result;
}

void Importer::LoadFbxBone(FbxCluster* Cluster, const int& BoneIndex, BoneTable& BoneTable)
{
	//該当ボーンが影響を与える頂点の数
	int pointNum = Cluster->GetControlPointIndicesCount();
	//影響を与える頂点のインデックス配列
	int* pointArray = Cluster->GetControlPointIndices();
	//ウェイト配列
	double* weightArray = Cluster->GetControlPointWeights();

	//ボーンが影響を与える頂点の情報を取得する
	for (int i = 0; i < pointNum; ++i)
	{
		//頂点インデックスとウェイトを取得
		int index = pointArray[i];
		float weight = (float)weightArray[i];

		//それらの情報を
		FbxBoneAffect info;
		info.index = BoneIndex;

		if (0.99 < weight)weight = 1.0f;
		if (weight < 0.01)weight = 0.0f;

		info.weight = weight;

		if (info.weight != 0.0f)
		{
			//頂点インデックス(添字)ごとにリストとして影響を受けるボーンは管理している。
			//ボーン情報をプッシュ
			BoneTable[index].emplace_front(info);
		}
	}
}

void Importer::LoadFbxSkin(Skeleton& Skel, FbxMesh* FbxMesh, BoneTable& BoneTable)
{
	//スキンの数を取得
	int skinCount = FbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	if (!skinCount)return;

	for (int i = 0; i < skinCount; ++i)
	{
		//i番目のスキンを取得
		FbxSkin* skin = (FbxSkin*)FbxMesh->GetDeformer(i, FbxDeformer::eSkin);

		//クラスターの数を取得
		int clusterNum = skin->GetClusterCount();

		for (int j = 0; j < clusterNum; ++j)
		{
			//j番目のクラスタを取得
			FbxCluster* cluster = skin->GetCluster(j);
			std::string clusterName = cluster->GetName();

			//ボーンを新規追加するかどうか
			bool newBone = true;
			int boneIdx = j;
			for (int k = 0; k < Skel.bones.size(); ++k)
			{
				//既にスケルトンに存在するボーンならそのインデックスをセット
				if (Skel.bones[k].name == clusterName)
				{
					newBone = false;
					boneIdx = k;
					break;
				}
			}
			//新しいボーンの追加
			if (newBone)
			{
				Skel.bones.emplace_back();
				Skel.bones.back().name = clusterName;
				boneIdx = Skel.bones.size() - 1;
			}

			//クラスターをボーンの情報として保存
			LoadFbxBone(cluster, boneIdx, BoneTable);

			//ボーンのオフセット行列
			FbxAMatrix globalTransform;
			cluster->GetTransformLinkMatrix(globalTransform);
			Skel.bones[j].offsetMat = FbxMatrixConvert(globalTransform);
		}
	}
}

void Importer::SetBoneAffectToVertex(Vertex& Vertex, const int& VertexIdx, BoneTable& BoneTable)
{
	//影響データ表が空じゃない
	if (!BoneTable[VertexIdx].empty())
	{
		//適用されるボーンの数
		int count = 0;

		//該当インデックスの影響データ一覧を参照
		for (auto itr = BoneTable[VertexIdx].begin(); itr != BoneTable[VertexIdx].end(); ++itr)
		{
			for (int i = 0; i < 4; ++i)
			{
				//対象の頂点のボーンデータで空な領域にデータを保存
				if (Vertex.boneIdx[i] == -1)	//ボーン未登録
				{
					Vertex.boneIdx[i] = itr->index;
					Vertex.boneWeight[i] = itr->weight;
					break;
				}
			}
			count++;
		}

		//４つまでしかボーン適用できない
		KuroFunc::ErrorMessage(4 < count, "Importer", "SetBoneAffectToVertex", "頂点読み込みにて５つ以上のボーンが適用されました。１つの頂点につき最大４つのボーンを適用出来ません\n");
	}
}

void Importer::LoadFbxVertex(ModelMesh& ModelMesh, FbxMesh* FbxMesh, BoneTable& BoneTable)
{
	//頂点バッファの取得
	FbxVector4* vertices = FbxMesh->GetControlPoints();
	//インデックスバッファの取得
	int* indices = FbxMesh->GetPolygonVertices();
	//頂点座標の数の取得
	int polygonVertexCount = FbxMesh->GetPolygonVertexCount();

	//UVデータの数
	int uvCount = FbxMesh->GetTextureUVCount();
	//uvsetの名前保存用
	FbxStringList uvsetNames;
	//UVSetの名前リストを取得
	FbxMesh->GetUVSetNames(uvsetNames);
	//UV取得
	FbxArray<FbxVector2>uvBuffers;
	FbxMesh->GetPolygonVertexUVs(uvsetNames.GetStringAt(0), uvBuffers);

	//法線取得
	FbxArray<FbxVector4> normals;
	FbxMesh->GetPolygonVertexNormals(normals);

	//頂点情報の取得
	for (int i = 0; i < polygonVertexCount; i++)
	{
		Vertex vertex;

		//インデックスバッファから頂点番号を取得
		int index = indices[i];

		//頂点座標リストから座標を取得
		vertex.pos.x = vertices[index][0];
		vertex.pos.y = vertices[index][1];
		vertex.pos.z = vertices[index][2];

		//法線リストから法線を取得
		vertex.normal.x = (float)normals[i][0];
		vertex.normal.y = (float)normals[i][1];
		vertex.normal.z = (float)normals[i][2];

		//UVリストから取得
		vertex.uv.x = (float)uvBuffers[i][0];
		vertex.uv.y = (float)uvBuffers[i][1];

		//保存しておいたボーンの対応表から頂点に対する影響データを取得
		SetBoneAffectToVertex(vertex, index, BoneTable);

		//モデルのメッシュに頂点追加
		ModelMesh.mesh->vertices.emplace_back(vertex);
	}
}

void Importer::LoadFbxIndex(ModelMesh& ModelMesh, FbxMesh* FbxMesh)
{
	//ポリゴンの数だけ連番として保存する
	for (int i = 0; i < FbxMesh->GetPolygonCount(); i++)
	{
		//左手系（右周り）
		ModelMesh.mesh->indices.emplace_back(i * 3 + 1);
		ModelMesh.mesh->indices.emplace_back(i * 3 + 2);
		ModelMesh.mesh->indices.emplace_back(i * 3);
	}
}

std::string Importer::GetFileName(std::string FbxRelativeFileName)
{
	auto strItr = FbxRelativeFileName.begin();

	for (auto itr = FbxRelativeFileName.begin(); itr != FbxRelativeFileName.end(); itr++)
	{
		if (*itr == '\\')
		{
			*itr = '/';
		}
		if (*itr == '/')
		{
			strItr = itr + 1;
		}
	}
	std::string result;
	for (auto itr = strItr; itr != FbxRelativeFileName.end(); itr++)
	{
		result += *itr;
	}

	return result;
}

void Importer::LoadFbxMaterial(const std::string& Dir, ModelMesh& ModelMesh, FbxMesh* FbxMesh)
{
	FbxNode* meshRootNode = FbxMesh->GetNode();
	//マテリアルの数
	int materialNum = meshRootNode->GetMaterialCount();

	//メッシュにアタッチされる予定のマテリアル名
	std::string attachedMatName;
	if (FbxMesh->GetElementMaterialCount() != 0)
	{
		FbxLayerElementMaterial* element = FbxMesh->GetElementMaterial(0);
		//(FBX上での)マテリアルのインデックス取得
		int materialIndexNum = element->GetIndexArray().GetAt(0);
		//(FBX上での)マテリアル取得
		FbxSurfaceMaterial* surface_material = FbxMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(materialIndexNum);
		//メッシュに適用するマテリアル名取得
		attachedMatName = surface_material->GetName();
	}

	// マテリアルがない or 名前割当がなければ何もしない
	if (materialNum == 0 || attachedMatName.empty())
	{
		//デフォルトのマテリアル生成
		ModelMesh.material = std::make_shared<Material>();
		return;
	}

	//マテリアルの情報を取得
	for (int i = 0; i < materialNum; ++i)
	{
		FbxSurfaceMaterial* material = meshRootNode->GetMaterial(i);

		if (material == nullptr)continue;
		if (material->GetName() != attachedMatName)continue;	//メッシュが要求するマテリアル名と異なればスルー

		//マテリアル解析
		std::shared_ptr<Material> newMaterial = std::make_shared<Material>();
		newMaterial->name = material->GetName();

		auto& info = newMaterial->constData;

		//Lambert
		FbxSurfaceLambert* lambert = (FbxSurfaceLambert*)material;

		//アンビエント
		info.ambient.x = (float)lambert->Ambient.Get()[0];
		info.ambient.y = (float)lambert->Ambient.Get()[1];
		info.ambient.z = (float)lambert->Ambient.Get()[2];
		info.ambientFactor = (float)lambert->AmbientFactor.Get();

		//ディフューズ
		info.diffuse.x = (float)lambert->Diffuse.Get()[0];
		info.diffuse.y = (float)lambert->Diffuse.Get()[1];
		info.diffuse.z = (float)lambert->Diffuse.Get()[2];
		info.diffuseFactor = (float)lambert->DiffuseFactor.Get();

		//放射
		info.emissive.x = (float)lambert->Emissive.Get()[0];
		info.emissive.y = (float)lambert->Emissive.Get()[1];
		info.emissive.z = (float)lambert->Emissive.Get()[2];
		info.emissiveFactor = (float)lambert->EmissiveFactor.Get();

		//透過度
		info.transparent = (float)lambert->TransparencyFactor.Get();

		//Phong
		if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

			//スペキュラー
			info.specular.x = (float)phong->Specular.Get()[0];
			info.specular.y = (float)phong->Specular.Get()[1];
			info.specular.z = (float)phong->Specular.Get()[2];
			info.specularFactor = (float)phong->SpecularFactor.Get();

			//光沢
			info.shininess = (float)phong->Shininess.Get();

			//反射
			info.reflection = (float)phong->ReflectionFactor.Get();

			//放射テクスチャ
			FbxFileTexture* emissiveTex = nullptr;
			int emissiveTexNum = phong->Emissive.GetSrcObjectCount();
			if (emissiveTexNum)
			{
				emissiveTex = phong->Emissive.GetSrcObject<FbxFileTexture>(0);
			}

			if (emissiveTex != nullptr)
			{
				auto path = Dir + GetFileName(emissiveTex->GetRelativeFileName());
				//エミッシブマップ
				newMaterial->textures[EMISSIVE_TEX].path = path;
				newMaterial->textures[EMISSIVE_TEX].texBuff = D3D12App::Instance()->GenerateTextureBuffer(path);
			}
		}

		//ディヒューズがテクスチャの情報を持っている
		auto prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		//FbxFileTextureを取得
		FbxFileTexture* tex = nullptr;
		int textureNum = prop.GetSrcObjectCount<FbxFileTexture>();
		if (0 < textureNum)
		{
			//propからFbxFileTextureを取得
			tex = prop.GetSrcObject<FbxFileTexture>(0);
		}
		else
		{
			//失敗したらマルチテクスチャの可能性を考えてFbxLayeredTextureを指定
			//FbxLayeredTextureからFbxFileTextureを取得
			int layerNum = prop.GetSrcObjectCount<FbxLayeredTexture>();
			if (0 < layerNum)
			{
				tex = prop.GetSrcObject<FbxFileTexture>(0);
			}
		}

		if (tex != nullptr)
		{
			auto path = Dir + GetFileName(tex->GetRelativeFileName());
			newMaterial->textures[COLOR_TEX].path = path;
			newMaterial->textures[COLOR_TEX].texBuff = D3D12App::Instance()->GenerateTextureBuffer(path);
		}

		ModelMesh.material = newMaterial;
		break;
	}
}

std::shared_ptr<Model> Importer::CheckAlreadyExsit(const std::string& Dir, const std::string& FileName)
{
	//生成済
	for (auto& m : models)
	{
		//既に生成しているものを渡す
		if (m.first == Dir + FileName)return m.second;
	}

	return std::shared_ptr<Model>();
}

std::shared_ptr<Model> Importer::LoadHSMModel(const std::string& Dir, const std::string& FileName, const std::string& Path)
{
	static const std::string FUNC_NAME = "LoadHSMModel";
	static const std::string MSG_TAIL = "の読み取りに失敗\n";
	std::shared_ptr<Model>model = std::make_shared<Model>(Dir, FileName);

	FILE* fp;
	fopen_s(&fp, Path.c_str(), "rb");
	ErrorMessage(FUNC_NAME, fp == nullptr, ".hsmファイルのバイナリ読み取りモードでのオープンに失敗\n");

	//最終更新日時（データ格納はしない）
	FILETIME dummy;
	ErrorMessage(FUNC_NAME, !LoadData(fp, &dummy, sizeof(dummy), 1), "最終更新日時(ダミー)" + MSG_TAIL);

	size_t size;

	//メッシュの数
	unsigned short meshNum;
	ErrorMessage(FUNC_NAME, !LoadData(fp, &meshNum, sizeof(meshNum), 1), "メッシュの数" + MSG_TAIL);

	//メッシュ生成
	model->meshes.resize(meshNum);

	//メッシュ情報
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		//メッシュ取得
		auto& mesh = model->meshes[meshIdx];
		mesh.mesh = std::make_shared<Mesh<ModelMesh::Vertex_Model>>();
		mesh.material = std::make_shared<Material>();

		//メッシュ名サイズ
		ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "メッシュ名サイズ" + MSG_TAIL);
		//メッシュ名
		mesh.mesh->name.resize(size);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &mesh.mesh->name[0], size, 1), "メッシュ名" + MSG_TAIL);

		//頂点数
		unsigned short vertNum;
		ErrorMessage(FUNC_NAME, !LoadData(fp, &vertNum, sizeof(vertNum), 1), "頂点数" + MSG_TAIL);
		//頂点情報
		mesh.mesh->vertices.resize(vertNum);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &mesh.mesh->vertices[0], sizeof(ModelMesh::Vertex_Model), vertNum), "頂点情報" + MSG_TAIL);

		//インデックス数
		unsigned short idxNum;
		ErrorMessage(FUNC_NAME, !LoadData(fp, &idxNum, sizeof(idxNum), 1), "インデックス数" + MSG_TAIL);
		//インデックス情報
		mesh.mesh->indices.resize(idxNum);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &mesh.mesh->indices[0], sizeof(unsigned int), idxNum), "インデックス情報" + MSG_TAIL);

		//マテリアル取得
		auto& material = mesh.material;
		//マテリアル名サイズ
		ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "マテリアル名サイズ" + MSG_TAIL);
		//マテリアル名
		material->name.resize(size);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &material->name[0], size, 1), "マテリアル名" + MSG_TAIL);
		//マテリアル情報
		ErrorMessage(FUNC_NAME, !LoadData(fp, &material->constData, sizeof(Material::ConstData), 1), "マテリアル情報" + MSG_TAIL);

		//画像ファイルテクスチャ保持フラグ
		unsigned char haveTex[MATERIAL_TEX_TYPE_NUM];
		ErrorMessage(FUNC_NAME, !LoadData(fp, haveTex, sizeof(unsigned char), MATERIAL_TEX_TYPE_NUM), "画像ファイルテクスチャ保持フラグ" + MSG_TAIL);
		//画像ファイルテクスチャパス情報
		for (int texIdx = 0; texIdx < MATERIAL_TEX_TYPE_NUM; ++texIdx)
		{
			if (!haveTex[texIdx])continue;

			//ファイルパスサイズ
			ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "ファイルパスサイズ" + MSG_TAIL);
			//ファイルパス
			material->textures[texIdx].path.resize(size);
			ErrorMessage(FUNC_NAME, !LoadData(fp, &material->textures[texIdx].path[0], size, 1), "画像ファイルパス" + MSG_TAIL);
		}

		mesh.material->CreateBuff();
		mesh.mesh->CreateBuff();
	}

	//スケルトン取得
	auto& skel = model->skelton;
	//ボーンの数
	unsigned short boneNum;
	ErrorMessage(FUNC_NAME, !LoadData(fp, &boneNum, sizeof(boneNum), 1), "ボーン数" + MSG_TAIL);
	skel.bones.resize(boneNum);

	//ボーン情報
	for (int boneIdx = 0; boneIdx < boneNum; ++boneIdx)
	{
		//ボーン取得
		auto& bone = skel.bones[boneIdx];
		//ボーン名サイズ
		ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "ボーン名サイズ" + MSG_TAIL);
		//ボーン名
		bone.name.resize(size);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &bone.name[0], size, 1), "ボーン名" + MSG_TAIL);

		//名前以外のボーン情報
		ErrorMessage(FUNC_NAME, !LoadData(fp, &bone.parent, Bone::GetSizeWithOutName(), 1), "ボーン情報" + MSG_TAIL);
	}
	skel.CreateBoneTree();

	fclose(fp);

	RegisterImportModel(Dir, FileName, model);

	return model;
}

void Importer::SaveHSMModel(const std::string& FileNameTail, std::shared_ptr<Model> Model, const FILETIME& LastWriteTime)
{
	static const std::string FUNC_NAME = "SaveHSMModel";
	static const std::string MSG_TAIL = "の書き込みに失敗\n";

	FILE* fp;
	fopen_s(&fp, (Model->header.dir + Model->header.GetModelName() + FileNameTail + ".hsm").c_str(), "wb");
	ErrorMessage(FUNC_NAME, fp == nullptr, ".hsmファイルのバイナリ書き込みモードでのオープンに失敗\n");

	//最終更新日時
	ErrorMessage(FUNC_NAME, !SaveData(fp, &LastWriteTime, sizeof(LastWriteTime), 1), "最終更新日時" + MSG_TAIL);

	size_t size;

	//メッシュの数
	unsigned short meshNum = Model->meshes.size();
	ErrorMessage(FUNC_NAME, !SaveData(fp, &meshNum, sizeof(meshNum), 1), "メッシュの数" + MSG_TAIL);

	//メッシュ情報
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		//メッシュ取得
		auto& mesh = Model->meshes[meshIdx];
		//メッシュ名サイズ
		size = mesh.mesh->name.length();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "メッシュ名サイズ" + MSG_TAIL);
		//メッシュ名
		ErrorMessage(FUNC_NAME, !SaveData(fp, mesh.mesh->name.data(), size, 1), "メッシュ名" + MSG_TAIL);

		//頂点数
		unsigned short vertNum = mesh.mesh->vertices.size();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &vertNum, sizeof(vertNum), 1), "頂点数" + MSG_TAIL);
		//頂点情報
		ErrorMessage(FUNC_NAME, !SaveData(fp, mesh.mesh->vertices.data(), sizeof(ModelMesh::Vertex_Model), vertNum), "頂点情報" + MSG_TAIL);

		//インデックス数
		unsigned short idxNum = mesh.mesh->indices.size();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &idxNum, sizeof(idxNum), 1), "インデックス数" + MSG_TAIL);
		//インデックス情報
		ErrorMessage(FUNC_NAME, !SaveData(fp, mesh.mesh->indices.data(), sizeof(unsigned int), idxNum), "インデックス情報" + MSG_TAIL);

		//マテリアル取得
		auto& material = mesh.material;
		//マテリアル名サイズ
		size = material->name.length();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "マテリアル名サイズ" + MSG_TAIL);
		//マテリアル名
		ErrorMessage(FUNC_NAME, !SaveData(fp, material->name.data(), size, 1), "マテリアル名" + MSG_TAIL);
		//マテリアル情報
		ErrorMessage(FUNC_NAME, !SaveData(fp, &material->constData, sizeof(Material::ConstData), 1), "マテリアル情報" + MSG_TAIL);

		//画像ファイルテクスチャ保持フラグ
		unsigned char haveTex[MATERIAL_TEX_TYPE_NUM] = { 0 };
		for (int texIdx = 0; texIdx < MATERIAL_TEX_TYPE_NUM; ++texIdx)
		{
			if (material->textures[texIdx].path.empty())continue;

			//テクスチャが割り当てられている
			haveTex[texIdx] = 1;
		}
		ErrorMessage(FUNC_NAME, !SaveData(fp, haveTex, sizeof(unsigned char), MATERIAL_TEX_TYPE_NUM), "マテリアルのテクスチャ保持フラグ" + MSG_TAIL);

		//画像ファイルテクスチャパス情報
		for(int texIdx = 0;texIdx < MATERIAL_TEX_TYPE_NUM;++texIdx)
		{
			if (!haveTex[texIdx])continue;

			//画像ファイルパスサイズ
			size = material->textures[texIdx].path.length();
			ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "テクスチャ画像ファイルパスサイズ" + MSG_TAIL);
			ErrorMessage(FUNC_NAME, !SaveData(fp, material->textures[texIdx].path.data(), size, 1), "テクスチャ画像ファイルパス" + MSG_TAIL);
		}
	}

	//スケルトン取得
	auto& skel = Model->skelton;
	//ボーンの数
	unsigned short boneNum = skel.bones.size();
	ErrorMessage(FUNC_NAME, !SaveData(fp, &boneNum, sizeof(boneNum), 1), "ボーン数" + MSG_TAIL);
	//ボーン情報
	for (int boneIdx = 0; boneIdx < boneNum; ++boneIdx)
	{
		//ボーン取得
		auto& bone = skel.bones[boneIdx];
		//ボーン名サイズ
		size = bone.name.length();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "ボーン名サイズ" + MSG_TAIL);
		//ボーン名
		ErrorMessage(FUNC_NAME, !SaveData(fp, bone.name.data(), size, 1), "ボーン名" + MSG_TAIL);
		//ボーン情報
		ErrorMessage(FUNC_NAME, !SaveData(fp, &bone.parent, Bone::GetSizeWithOutName(), 1), "ボーン情報" + MSG_TAIL);
	}

	fclose(fp);
}

Importer::Importer()
{
	static const std::string FUNC_NAME = "コンストラクタ";

	//マネージャ生成
	fbxManager = FbxManager::Create();
	ErrorMessage(FUNC_NAME, fbxManager == nullptr, "FBXマネージャ生成に失敗\n");

	//IOSettingを生成
	ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	ErrorMessage(FUNC_NAME, ioSettings == nullptr, "FBXIOSetting生成に失敗\n");

	//インポータ生成
	fbxImporter = FbxImporter::Create(fbxManager, "");
	ErrorMessage(FUNC_NAME, fbxImporter == nullptr, "FBXインポータ生成に失敗\n");
}

std::shared_ptr<Model> Importer::LoadFBXModel(const std::string& Dir, const std::string& FileName)
{
	static const std::string FUNC_NAME = "LoadFBX";
	static const std::string HSM_TAIL = "_fbx";

	std::shared_ptr<Model>result;

	//既に生成しているか
	result = CheckAlreadyExsit(Dir, FileName);
	if (result)return result;	//生成していたらそれを返す

	//拡張子取得
	const auto ext = "." + KuroFunc::GetExtension(FileName);
	ErrorMessage(FUNC_NAME, ext != ".fbx", "拡張子が合いません\n");

	//モデル名取得(ファイル名から拡張子を除いたもの)
	auto modelName = FileName;
	modelName.erase(modelName.size() - ext.size());

	//ファイルパス
	const auto path = Dir + FileName;

	//fbxファイルの最終更新日時を読み取る
	FILETIME fbxLastWriteTime;
	HANDLE fbxFile = CreateFile(
		KuroFunc::GetWideStrFromStr(path).c_str(),
		0,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	ErrorMessage(FUNC_NAME, !GetFileTime(fbxFile, NULL, NULL, &fbxLastWriteTime), "fbxファイルの最終更新日時読み取りに失敗\n");

	//hsmファイルが存在するか
	const auto hsmPath = Dir + modelName + HSM_TAIL + ".hsm";
	if (canLoadHSM && KuroFunc::ExistFile(hsmPath))
	{
		//hsmファイルに記録されたモデルの最終更新日時データを読み取る
		FILETIME modelLastWriteTime;
		FILE* fp;
		fopen_s(&fp, hsmPath.c_str(), "rb");
		if (fp != NULL)
		{
			ErrorMessage(FUNC_NAME, fread(&modelLastWriteTime, sizeof(FILETIME), 1, fp) < 1, "hsmファイルの最終更新日時読み取りに失敗\n");
			fclose(fp);
		}

		//fbxファイルの最終更新日時と比較、同じならhsmファイルをロードしてそれを返す
		if (modelLastWriteTime.dwHighDateTime == fbxLastWriteTime.dwHighDateTime
			&& modelLastWriteTime.dwLowDateTime == fbxLastWriteTime.dwLowDateTime)
		{
			return LoadHSMModel(Dir, FileName, hsmPath);
		}
	}


	//ファイルの存在を確認
	ErrorMessage(FUNC_NAME, !KuroFunc::ExistFile(path), "ファイルが存在しません\n");

	//ファイルを初期化する
	ErrorMessage(FUNC_NAME, fbxImporter->Initialize(path.c_str(), -1, fbxManager->GetIOSettings()) == false, "ファイルの初期化に失敗\n");

	//シーンオブジェクト生成
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
	ErrorMessage(FUNC_NAME, fbxScene == nullptr, "FBXシーン生成に失敗\n");

	//シーンオブジェクトにfbxファイル内の情報を流し込む
	ErrorMessage(FUNC_NAME, fbxImporter->Import(fbxScene) == false, "FBXシーンへのFBXファイル情報流し込みに失敗\n");

	//シーン内のノードのポリゴンを全て三角形にする
	FbxGeometryConverter converter(fbxManager);

	//全FbxMeshをマテリアル単位で分割
	converter.SplitMeshesPerMaterial(fbxScene, true);

	//ポリゴンを三角形にする
	converter.Triangulate(fbxScene, true);

	//メッシュ情報読み取り、保存
	std::vector<FbxMesh*>fbxMeshes;
	TraceFbxMesh(fbxScene->GetRootNode(), &fbxMeshes);

	result = std::make_shared<Model>(Dir, FileName);
	Skeleton skel;

	for (int i = 0; i < fbxMeshes.size(); ++i)
	{
		auto& fbxMesh = fbxMeshes[i];

		//モデル用メッシュ生成
		ModelMesh mesh;
		mesh.mesh = std::make_shared<Mesh<ModelMesh::Vertex_Model>>();

		//メッシュ名取得
		const std::string meshName = fbxMesh->GetName();

		//メッシュ名セット
		mesh.mesh->name = meshName;

		//ボーンが頂点に与える影響に関する情報テーブル
		BoneTable boneAffectTable;

		//ボーン（スケルトン）
		LoadFbxSkin(skel, fbxMesh, boneAffectTable);
		//頂点
		LoadFbxVertex(mesh, fbxMesh, boneAffectTable);
		//インデックス
		LoadFbxIndex(mesh, fbxMesh);
		//マテリアル
		LoadFbxMaterial(Dir, mesh, fbxMesh);

		mesh.material->CreateBuff();
		mesh.mesh->CreateBuff();

		result->meshes.emplace_back(mesh);
	}

	skel.CreateBoneTree();
	result->skelton = skel;

	SaveHSMModel(HSM_TAIL, result, fbxLastWriteTime);

	RegisterImportModel(Dir, FileName, result);

	return result;
}