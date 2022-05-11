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
		vertex.pos.x = -vertices[index][0];
		vertex.pos.y = vertices[index][1];
		vertex.pos.z = vertices[index][2];

		//法線リストから法線を取得
		vertex.normal.x = -(float)normals[i][0];
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
		ModelMesh.mesh->indices.emplace_back(i * 3);
		ModelMesh.mesh->indices.emplace_back(i * 3 + 2);
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
		info.lambert.ambient.x = (float)lambert->Ambient.Get()[0];
		info.lambert.ambient.y = (float)lambert->Ambient.Get()[1];
		info.lambert.ambient.z = (float)lambert->Ambient.Get()[2];
		info.lambert.ambientFactor = (float)lambert->AmbientFactor.Get();

		//ディフューズ
		info.lambert.diffuse.x = (float)lambert->Diffuse.Get()[0];
		info.lambert.diffuse.y = (float)lambert->Diffuse.Get()[1];
		info.lambert.diffuse.z = (float)lambert->Diffuse.Get()[2];
		info.lambert.diffuseFactor = (float)lambert->DiffuseFactor.Get();

		//放射
		info.lambert.emissive.x = (float)lambert->Emissive.Get()[0];
		info.lambert.emissive.y = (float)lambert->Emissive.Get()[1];
		info.lambert.emissive.z = (float)lambert->Emissive.Get()[2];
		info.lambert.emissiveFactor = (float)lambert->EmissiveFactor.Get();

		//透過度
		info.lambert.transparent = (float)lambert->TransparencyFactor.Get();

		//Phong
		if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

			//スペキュラー
			info.phong.specular.x = (float)phong->Specular.Get()[0];
			info.phong.specular.y = (float)phong->Specular.Get()[1];
			info.phong.specular.z = (float)phong->Specular.Get()[2];
			info.phong.specularFactor = (float)phong->SpecularFactor.Get();

			//光沢
			info.phong.shininess = (float)phong->Shininess.Get();

			//反射
			info.phong.reflection = (float)phong->ReflectionFactor.Get();

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
				newMaterial->texBuff[EMISSIVE_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
			}
		}

		//PBR
		//ベースカラー
		{
			const auto propBaseColor = FbxSurfaceMaterialUtils::GetProperty("baseColor", material);
			if (propBaseColor.IsValid())
			{
				const FbxFileTexture* baseColorTex = propBaseColor.GetSrcObject<FbxFileTexture>();

				if (baseColorTex)
				{
					auto path = Dir + GetFileName(baseColorTex->GetRelativeFileName());
					newMaterial->texBuff[BASE_COLOR_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
				}
				else
				{
					auto baseCol = propBaseColor.Get<FbxDouble3>();
					newMaterial->constData.pbr.baseColor.x = (float)baseCol.Buffer()[0];
					newMaterial->constData.pbr.baseColor.y = (float)baseCol.Buffer()[1];
					newMaterial->constData.pbr.baseColor.z = (float)baseCol.Buffer()[2];
				}
			}
		}
		//金属度
		{
			const auto propMetalness = FbxSurfaceMaterialUtils::GetProperty("metalness", material);
			if (propMetalness.IsValid())
			{
				const FbxFileTexture* metalnessTex = propMetalness.GetSrcObject<FbxFileTexture>();

				if (metalnessTex)
				{
					auto path = Dir + GetFileName(metalnessTex->GetRelativeFileName());
					newMaterial->texBuff[METALNESS_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
				}
				else 
				{
					newMaterial->constData.pbr.metalness = propMetalness.Get<float>();
				}
			}
		}
		//PBRスペキュラー
		const auto propSpecular = FbxSurfaceMaterialUtils::GetProperty("specular", material);
		if (propSpecular.IsValid())
		{
			newMaterial->constData.pbr.specular = propSpecular.Get<float>();
		}
		//粗さ
		{
			const auto propRoughness = FbxSurfaceMaterialUtils::GetProperty("specularRoughness", material);
			if (propRoughness.IsValid())
			{
				const FbxFileTexture* roughnessTex = propRoughness.GetSrcObject<FbxFileTexture>();

				if (roughnessTex)
				{
					auto path = Dir + GetFileName(roughnessTex->GetRelativeFileName());
					newMaterial->texBuff[ROUGHNESS_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
				}
				else
				{
					newMaterial->constData.pbr.roughness = propRoughness.Get<float>();
				}
			}
		}

		//ディヒューズがテクスチャの情報を持っている
		{
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
				const auto fileName = GetFileName(tex->GetRelativeFileName());
				if (!fileName.empty())
				{
					auto path = Dir + fileName;
					newMaterial->texBuff[COLOR_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
				}
			}
		}

		//法線マップ
		const FbxProperty propNormalCamera = FbxSurfaceMaterialUtils::GetProperty("normalCamera", material);
		if (propNormalCamera.IsValid())
		{
			const FbxFileTexture* normalTex = propNormalCamera.GetSrcObject<FbxFileTexture>();
			if (normalTex)
			{
				auto path = Dir + GetFileName(normalTex->GetRelativeFileName());
				newMaterial->texBuff[NORMAL_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
			}
		}

		ModelMesh.material = newMaterial;
		break;
	}
}

void Importer::TraceBoneAnim(Skeleton::ModelAnimation& ModelAnimation, FbxNode* FbxNode, FbxAnimLayer* FbxAnimLayer)
{
	auto attribute = FbxNode->GetNodeAttribute();

	//ボーンのアニメーションしか読み込まない
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		std::string boneName = FbxNode->GetName();

		//ボーン単位アニメーション取得
		auto& boneAnim = ModelAnimation.boneAnim[boneName];

		FbxAnimCurve* animCurve;

		//座標
		animCurve = FbxNode->LclTranslation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animCurve)LoadAnimCurve(animCurve, boneAnim.pos[Skeleton::BoneAnimation::X]);

		animCurve = FbxNode->LclTranslation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.pos[Skeleton::BoneAnimation::Y]);

		animCurve = FbxNode->LclTranslation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.pos[Skeleton::BoneAnimation::Z]);

		//回転
		animCurve = FbxNode->LclRotation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animCurve)LoadAnimCurve(animCurve, boneAnim.rotate[Skeleton::BoneAnimation::X]);

		animCurve = FbxNode->LclRotation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.rotate[Skeleton::BoneAnimation::Y]);

		animCurve = FbxNode->LclRotation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.rotate[Skeleton::BoneAnimation::Z]);

		//スケール
		animCurve = FbxNode->LclScaling.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animCurve)LoadAnimCurve(animCurve, boneAnim.scale[Skeleton::BoneAnimation::X]);

		animCurve = FbxNode->LclScaling.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.scale[Skeleton::BoneAnimation::Y]);

		animCurve = FbxNode->LclScaling.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.scale[Skeleton::BoneAnimation::Z]);
	}

	// 子ノードに対して再帰呼び出し
	for (int i = 0; i < FbxNode->GetChildCount(); i++) {
		TraceBoneAnim(ModelAnimation, FbxNode->GetChild(i), FbxAnimLayer);
	}
}

void Importer::LoadAnimCurve(FbxAnimCurve* FbxAnimCurve, Animation& Animation)
{
	FbxTimeSpan interval;

	static const auto ONE_FRAME_VALUE = FbxTime::GetOneFrameValue(FbxTime::eFrames60);


	if (FbxAnimCurve->GetTimeInterval(interval)) {
		FbxLongLong start = interval.GetStart().Get();
		FbxLongLong end = interval.GetStop().Get();
		Animation.startFrame = start / ONE_FRAME_VALUE;
		Animation.endFrame =  end / ONE_FRAME_VALUE;
	}

	int lKeyCount = FbxAnimCurve->KeyGetCount();

	int lCount;

	for (lCount = 0; lCount < lKeyCount; lCount++)
	{
		float lKeyValue = static_cast<float>(FbxAnimCurve->KeyGetValue(lCount));
		FbxTime lKeyTime = FbxAnimCurve->KeyGetTime(lCount);

		KeyFrame keyFrame{};

		keyFrame.frame = lKeyTime.Get() / ONE_FRAME_VALUE;
		keyFrame.value = lKeyValue;

		Animation.keyFrames.emplace_back(keyFrame);
	}
}

void Importer::LoadGLTFPrimitive(ModelMesh& ModelMesh, const Microsoft::glTF::MeshPrimitive& GLTFPrimitive, const Microsoft::glTF::GLTFResourceReader& Reader, const Microsoft::glTF::Document& Doc)
{
	using namespace Microsoft::glTF;

	// 頂点位置情報アクセッサの取得
	auto& idPos = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_POSITION);
	auto& accPos = Doc.accessors.Get(idPos);
	auto vertPos = Reader.ReadBinaryData<float>(Doc, accPos);
	// 法線情報アクセッサの取得
	auto& idNrm = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_NORMAL);
	auto& accNrm = Doc.accessors.Get(idNrm);
	auto vertNrm = Reader.ReadBinaryData<float>(Doc, accNrm);
	// テクスチャ座標情報アクセッサの取得
	auto& idUV = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_TEXCOORD_0);
	auto& accUV = Doc.accessors.Get(idUV);
	auto vertUV = Reader.ReadBinaryData<float>(Doc, accUV);

	std::vector<uint8_t>vertJoint;
	if (GLTFPrimitive.HasAttribute(ACCESSOR_JOINTS_0))
	{
		auto& idJoint = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_JOINTS_0);
		auto& accJoint = Doc.accessors.Get(idJoint);
		vertJoint = Reader.ReadBinaryData<uint8_t>(Doc, accJoint);
	}

	std::vector<float>vertWeight;
	if (GLTFPrimitive.HasAttribute(ACCESSOR_WEIGHTS_0))
	{
		auto& idWeight = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_WEIGHTS_0);
		auto& accWeight = Doc.accessors.Get(idWeight);
		vertWeight = Reader.ReadBinaryData<float>(Doc, accWeight);
	}

	auto vertexCount = accPos.count;
	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		// 頂点データの構築
		int vid0 = 3 * i, vid1 = 3 * i + 1, vid2 = 3 * i + 2;
		int tid0 = 2 * i, tid1 = 2 * i + 1;
		int jid0 = 4 * i, jid1 = 4 * i + 1, jid2 = 4 * i + 2, jid3 = 4 * i + 3;

		ModelMesh::Vertex_Model vertex;
		vertex.pos = { vertPos[vid0],vertPos[vid1],vertPos[vid2] };
		vertex.normal = { vertNrm[vid0],vertNrm[vid1],vertNrm[vid2] };
		vertex.uv = { vertUV[tid0],vertUV[tid1] };

		if (!vertJoint.empty())
		{
			if (vertex.boneWeight.x)vertex.boneIdx.x = static_cast<signed short>(vertJoint[jid0]);
			if (vertex.boneWeight.y)vertex.boneIdx.y = static_cast<signed short>(vertJoint[jid1]);
			if (vertex.boneWeight.z)vertex.boneIdx.z = static_cast<signed short>(vertJoint[jid2]);
			if (vertex.boneWeight.w)vertex.boneIdx.w = static_cast<signed short>(vertJoint[jid3]);
		}
		if (!vertWeight.empty())
		{
			vertex.boneWeight = { vertWeight[jid0],vertWeight[jid1],vertWeight[jid2],vertWeight[jid3] };
		}
		ModelMesh.mesh->vertices.emplace_back(vertex);
	}

	// 頂点インデックス用アクセッサの取得
	auto& idIndex = GLTFPrimitive.indicesAccessorId;
	auto& accIndex = Doc.accessors.Get(idIndex);

	// インデックスデータ
	std::vector<uint16_t>indices = Reader.ReadBinaryData<uint16_t>(Doc, accIndex);
	for (const auto& index : indices)
	{
		ModelMesh.mesh->indices.emplace_back(static_cast<unsigned int>(index));
	}
}

void Importer::PrintDocumentInfo(const Microsoft::glTF::Document& document)
{
	// Asset Info
	std::cout << "Asset Version:    " << document.asset.version << "\n";
	std::cout << "Asset MinVersion: " << document.asset.minVersion << "\n";
	std::cout << "Asset Generator:  " << document.asset.generator << "\n";
	std::cout << "Asset Copyright:  " << document.asset.copyright << "\n\n";

	// Scene Info
	std::cout << "Scene Count: " << document.scenes.Size() << "\n";

	if (document.scenes.Size() > 0U)
	{
		std::cout << "Default Scene Index: " << document.GetDefaultScene().id << "\n\n";
	}
	else
	{
		std::cout << "\n";
	}

	// Entity Info
	std::cout << "Node Count:     " << document.nodes.Size() << "\n";
	std::cout << "Camera Count:   " << document.cameras.Size() << "\n";
	std::cout << "Material Count: " << document.materials.Size() << "\n\n";

	// Mesh Info
	std::cout << "Mesh Count: " << document.meshes.Size() << "\n";
	std::cout << "Skin Count: " << document.skins.Size() << "\n\n";

	// Texture Info
	std::cout << "Image Count:   " << document.images.Size() << "\n";
	std::cout << "Texture Count: " << document.textures.Size() << "\n";
	std::cout << "Sampler Count: " << document.samplers.Size() << "\n\n";

	// Buffer Info
	std::cout << "Buffer Count:     " << document.buffers.Size() << "\n";
	std::cout << "BufferView Count: " << document.bufferViews.Size() << "\n";
	std::cout << "Accessor Count:   " << document.accessors.Size() << "\n\n";

	// Animation Info
	std::cout << "Animation Count: " << document.animations.Size() << "\n\n";

	for (const auto& extension : document.extensionsUsed)
	{
		std::cout << "Extension Used: " << extension << "\n";
	}

	if (!document.extensionsUsed.empty())
	{
		std::cout << "\n";
	}

	for (const auto& extension : document.extensionsRequired)
	{
		std::cout << "Extension Required: " << extension << "\n";
	}

	if (!document.extensionsRequired.empty())
	{
		std::cout << "\n";
	}
}

void Importer::PrintResourceInfo(const Microsoft::glTF::Document& document, const Microsoft::glTF::GLTFResourceReader& resourceReader)
{
	using namespace Microsoft::glTF;
	// Use the resource reader to get each mesh primitive's position data
	for (const auto& mesh : document.meshes.Elements())
	{
		std::cout << "Mesh: " << mesh.id << "\n";

		for (const auto& meshPrimitive : mesh.primitives)
		{
			std::string accessorId;

			if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_POSITION, accessorId))
			{
				const Accessor& accessor = document.accessors.Get(accessorId);

				const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
				const auto dataByteLength = data.size() * sizeof(float);

				std::cout << "MeshPrimitive: " << dataByteLength << " bytes of position data\n";
			}
		}

		std::cout << "\n";
	}

	// Use the resource reader to get each image's data
	for (const auto& image : document.images.Elements())
	{
		std::string filename;

		if (image.uri.empty())
		{
			assert(!image.bufferViewId.empty());

			auto& bufferView = document.bufferViews.Get(image.bufferViewId);
			auto& buffer = document.buffers.Get(bufferView.bufferId);

			filename += buffer.uri; //NOTE: buffer uri is empty if image is stored in GLB binary chunk
		}
		else if (IsUriBase64(image.uri))
		{
			filename = "Data URI";
		}
		else
		{
			filename = image.uri;
		}

		auto data = resourceReader.ReadBinaryData(document, image);

		std::cout << "Image: " << image.id << "\n";
		std::cout << "Image: " << data.size() << " bytes of image data\n";

		if (!filename.empty())
		{
			std::cout << "Image filename: " << filename << "\n\n";
		}
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

/*
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
*/

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
	printf("glTFロード\nDir : %s , FileName : %s\n", Dir.c_str(), FileName.c_str());

	static const std::string FUNC_NAME = "LoadFBX";
	static const std::string HSM_TAIL = "_fbx";

	std::shared_ptr<Model>result;

	//既に生成しているか
	result = CheckAlreadyExsit(Dir, FileName);
	if (result)return result;	//生成していたらそれを返す

	//拡張子取得
	const auto ext = "." + KuroFunc::GetExtension(FileName);
	ErrorMessage(FUNC_NAME, ext != ".fbx" && ext != ".FBX", "拡張子が合いません\n");

	//モデル名取得(ファイル名から拡張子を除いたもの)
	auto modelName = FileName;
	modelName.erase(modelName.size() - ext.size());

	//ファイルパス
	const auto path = Dir + FileName;

	/*
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
	*/

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

		mesh.BuildTangentAndBiNormal();
		mesh.material->CreateBuff();
		mesh.mesh->CreateBuff();

		result->meshes.emplace_back(mesh);
	}

	//アニメーションの数
	int animStackCount = fbxImporter->GetAnimStackCount();
	for (int animIdx = 0; animIdx < animStackCount; ++animIdx)
	{
		Skeleton::ModelAnimation animation;

		// AnimStack読み込み（AnimLayerの集合)
		FbxAnimStack* animStack = fbxScene->GetSrcObject<FbxAnimStack>(animIdx);
		animation.name = animStack->GetName();	//アニメーション名取得

		FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
		TraceBoneAnim(animation, fbxScene->GetRootNode(), animLayer);

		skel.animations.emplace_back(animation);

		////アニメーションが割り当てられている”部位”の数（AnimCurve の集合）
		//int animLayersCount = animStack->GetMemberCount<FbxAnimLayer>();
		//for (int i = 0; i < animLayersCount; ++i)
		//{
		//	FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(i);
		//	std::string layerName = animLayer->GetName();
		//	TraceBoneAnim(skel, fbxScene->GetRootNode(), animLayer);
		//}
	}

	skel.CreateBoneTree();
	result->skelton = skel;

	//SaveHSMModel(HSM_TAIL, result, fbxLastWriteTime);

	RegisterImportModel(Dir, FileName, result);

	return result;
}

#include<sstream>
std::shared_ptr<Model> Importer::LoadGLTFModel(const std::string& Dir, const std::string& FileName)
{
	printf("glTFロード\nDir : %s , FileName : %s\n", Dir.c_str(), FileName.c_str());
	std::shared_ptr<Model>result = std::make_shared<Model>(Dir, FileName);

	auto modelFilePath = std::experimental::filesystem::path(Dir + FileName);
	if (modelFilePath.is_relative())
	{
		auto current = std::experimental::filesystem::current_path();
		current /= modelFilePath;
		current.swap(modelFilePath);
	}
	auto reader = std::make_unique<StreamReader>(modelFilePath.parent_path());
	auto stream = reader->GetInputStream(modelFilePath.filename().u8string());

	//拡張子 ".gltf" ".glb" の判定
	std::experimental::filesystem::path pathFile = modelFilePath.filename();
	std::experimental::filesystem::path pathFileExt = pathFile.extension();

	std::string manifest;

	auto MakePathExt = [](const std::string& ext)
	{
		return "." + ext;
	};

	std::unique_ptr<Microsoft::glTF::GLTFResourceReader> resourceReader;

	if (pathFileExt == MakePathExt(Microsoft::glTF::GLTF_EXTENSION))
	{
		auto gltfResourceReader = std::make_unique<Microsoft::glTF::GLTFResourceReader>(std::move(reader));

		std::stringstream manifestStream;

		// Read the contents of the glTF file into a string using a std::stringstream
		manifestStream << stream->rdbuf();
		manifest = manifestStream.str();

		resourceReader = std::move(gltfResourceReader);
	}
	else if (pathFileExt == MakePathExt(Microsoft::glTF::GLB_EXTENSION))
	{
		auto glbResourceReader = std::make_unique<Microsoft::glTF::GLBResourceReader>(std::move(reader), std::move(stream));

		manifest = glbResourceReader->GetJson(); // Get the manifest from the JSON chunk

		resourceReader = std::move(glbResourceReader);
	}

	KuroFunc::ErrorMessage(!resourceReader, "Importer", "LoadGLTFModel", "ファイルの拡張子が glb でも gltf でもありません\n");

	Microsoft::glTF::Document doc;
	try
	{
		doc = Microsoft::glTF::Deserialize(manifest);
	}
	catch (const Microsoft::glTF::GLTFException& ex)
	{
		std::stringstream ss;
		ss << "Microsoft::glTF::Deserialize failed: ";
		ss << ex.what();
		throw std::runtime_error(ss.str());
	}

	PrintDocumentInfo(doc);
	PrintResourceInfo(doc, *resourceReader);

	//スケルトン読み込み途中
	/*
	for (const auto& glTFSkin : doc.skins.Elements())
	{
		auto& idBoneOffsetMat = glTFSkin.inverseBindMatricesAccessorId;
		auto& accBoneOffsetMat = doc.accessors.Get(idBoneOffsetMat);
		auto loadOffsetMat = resourceReader->ReadBinaryData<float>(doc, accBoneOffsetMat);
		int boneNum = accBoneOffsetMat.count;
		for (int i = 0; i < boneNum; ++i)
		{
			XMMATRIX offsetMatrix;
			for (int j = 0; j < 4; ++j)
			{
				const int idOffset = 4 * 4 * i + 4 * j;
				offsetMatrix.r[j].m128_f32[0] = loadOffsetMat[idOffset];
				offsetMatrix.r[j].m128_f32[1] = loadOffsetMat[idOffset + 1];
				offsetMatrix.r[j].m128_f32[2] = loadOffsetMat[idOffset + 2];
				offsetMatrix.r[j].m128_f32[3] = loadOffsetMat[idOffset + 3];
			}
			int asdas = 0;
		}
	}
	*/

	//マテリアル読み込み
	std::vector<std::shared_ptr<Material>>loadMaterials;
	for (auto& m : doc.materials.Elements())
	{
		auto material = std::make_shared<Material>();

		//PBR
		const auto baseColor = m.metallicRoughness.baseColorFactor;
		material->constData.pbr.baseColor.x = baseColor.r;
		material->constData.pbr.baseColor.y = baseColor.g;
		material->constData.pbr.baseColor.z = baseColor.b;
		material->constData.pbr.metalness = m.metallicRoughness.metallicFactor;
		material->constData.pbr.roughness = m.metallicRoughness.roughnessFactor;

		//カラーテクスチャ
		auto textureId = m.metallicRoughness.baseColorTexture.textureId;
		if (textureId.empty())textureId = m.normalTexture.textureId;

		if (!textureId.empty())
		{
			auto& texture = doc.textures.Get(textureId);
			auto& image = doc.images.Get(texture.imageId);

			//テクスチャ画像ファイル読み込み
			if (!image.uri.empty())
			{
				std::string path = Dir + image.uri;
				material->texBuff[COLOR_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
			}
			//テクスチャ画像がgltfに埋め込まれている
			else if (!image.bufferViewId.empty())
			{
				auto imageBufferView = doc.bufferViews.Get(image.bufferViewId);
				auto imageData = resourceReader->ReadBinaryData<char>(doc, imageBufferView);
				std::string path = "glTF - Load (" + image.mimeType + ") - " + image.name;
				material->texBuff[COLOR_TEX] = D3D12App::Instance()->GenerateTextureBuffer(imageData);
			}
		}

		material->CreateBuff();
		loadMaterials.emplace_back(material);
	}

	for (const auto& glTFMesh : doc.meshes.Elements())
	{
		for (const auto& meshPrimitive : glTFMesh.primitives)
		{
			//モデル用メッシュ生成
			ModelMesh mesh;
			mesh.mesh = std::make_shared<Mesh<ModelMesh::Vertex_Model>>();

			//頂点 & インデックス情報
			LoadGLTFPrimitive(mesh, meshPrimitive, *resourceReader, doc);

			int materialIdx = int(doc.materials.GetIndex(meshPrimitive.materialId));
			mesh.material = loadMaterials[materialIdx];

			mesh.BuildTangentAndBiNormal();
			mesh.mesh->CreateBuff();
			result->meshes.emplace_back(mesh);
		}
	}

	return result;
}
