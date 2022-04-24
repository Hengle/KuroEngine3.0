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
		FbxNode* child = Node->GetChild(i);	//�q�m�[�h���擾
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
	//�Y���{�[�����e����^���钸�_�̐�
	int pointNum = Cluster->GetControlPointIndicesCount();
	//�e����^���钸�_�̃C���f�b�N�X�z��
	int* pointArray = Cluster->GetControlPointIndices();
	//�E�F�C�g�z��
	double* weightArray = Cluster->GetControlPointWeights();

	//�{�[�����e����^���钸�_�̏����擾����
	for (int i = 0; i < pointNum; ++i)
	{
		//���_�C���f�b�N�X�ƃE�F�C�g���擾
		int index = pointArray[i];
		float weight = (float)weightArray[i];

		//�����̏���
		FbxBoneAffect info;
		info.index = BoneIndex;

		if (0.99 < weight)weight = 1.0f;
		if (weight < 0.01)weight = 0.0f;

		info.weight = weight;

		if (info.weight != 0.0f)
		{
			//���_�C���f�b�N�X(�Y��)���ƂɃ��X�g�Ƃ��ĉe�����󂯂�{�[���͊Ǘ����Ă���B
			//�{�[�������v�b�V��
			BoneTable[index].emplace_front(info);
		}
	}
}

void Importer::LoadFbxSkin(Skeleton& Skel, FbxMesh* FbxMesh, BoneTable& BoneTable)
{
	//�X�L���̐����擾
	int skinCount = FbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	if (!skinCount)return;

	for (int i = 0; i < skinCount; ++i)
	{
		//i�Ԗڂ̃X�L�����擾
		FbxSkin* skin = (FbxSkin*)FbxMesh->GetDeformer(i, FbxDeformer::eSkin);

		//�N���X�^�[�̐����擾
		int clusterNum = skin->GetClusterCount();

		for (int j = 0; j < clusterNum; ++j)
		{
			//j�Ԗڂ̃N���X�^���擾
			FbxCluster* cluster = skin->GetCluster(j);
			std::string clusterName = cluster->GetName();

			//�{�[����V�K�ǉ����邩�ǂ���
			bool newBone = true;
			int boneIdx = j;
			for (int k = 0; k < Skel.bones.size(); ++k)
			{
				//���ɃX�P���g���ɑ��݂���{�[���Ȃ炻�̃C���f�b�N�X���Z�b�g
				if (Skel.bones[k].name == clusterName)
				{
					newBone = false;
					boneIdx = k;
					break;
				}
			}
			//�V�����{�[���̒ǉ�
			if (newBone)
			{
				Skel.bones.emplace_back();
				Skel.bones.back().name = clusterName;
				boneIdx = Skel.bones.size() - 1;
			}

			//�N���X�^�[���{�[���̏��Ƃ��ĕۑ�
			LoadFbxBone(cluster, boneIdx, BoneTable);

			//�{�[���̃I�t�Z�b�g�s��
			FbxAMatrix globalTransform;
			cluster->GetTransformLinkMatrix(globalTransform);
			Skel.bones[j].offsetMat = FbxMatrixConvert(globalTransform);
		}
	}
}

void Importer::SetBoneAffectToVertex(Vertex& Vertex, const int& VertexIdx, BoneTable& BoneTable)
{
	//�e���f�[�^�\���󂶂�Ȃ�
	if (!BoneTable[VertexIdx].empty())
	{
		//�K�p�����{�[���̐�
		int count = 0;

		//�Y���C���f�b�N�X�̉e���f�[�^�ꗗ���Q��
		for (auto itr = BoneTable[VertexIdx].begin(); itr != BoneTable[VertexIdx].end(); ++itr)
		{
			for (int i = 0; i < 4; ++i)
			{
				//�Ώۂ̒��_�̃{�[���f�[�^�ŋ�ȗ̈�Ƀf�[�^��ۑ�
				if (Vertex.boneIdx[i] == -1)	//�{�[�����o�^
				{
					Vertex.boneIdx[i] = itr->index;
					Vertex.boneWeight[i] = itr->weight;
					break;
				}
			}
			count++;
		}

		//�S�܂ł����{�[���K�p�ł��Ȃ�
		KuroFunc::ErrorMessage(4 < count, "Importer", "SetBoneAffectToVertex", "���_�ǂݍ��݂ɂĂT�ȏ�̃{�[�����K�p����܂����B�P�̒��_�ɂ��ő�S�̃{�[����K�p�o���܂���\n");
	}
}

void Importer::LoadFbxVertex(ModelMesh& ModelMesh, FbxMesh* FbxMesh, BoneTable& BoneTable)
{
	//���_�o�b�t�@�̎擾
	FbxVector4* vertices = FbxMesh->GetControlPoints();
	//�C���f�b�N�X�o�b�t�@�̎擾
	int* indices = FbxMesh->GetPolygonVertices();
	//���_���W�̐��̎擾
	int polygonVertexCount = FbxMesh->GetPolygonVertexCount();

	//UV�f�[�^�̐�
	int uvCount = FbxMesh->GetTextureUVCount();
	//uvset�̖��O�ۑ��p
	FbxStringList uvsetNames;
	//UVSet�̖��O���X�g���擾
	FbxMesh->GetUVSetNames(uvsetNames);
	//UV�擾
	FbxArray<FbxVector2>uvBuffers;
	FbxMesh->GetPolygonVertexUVs(uvsetNames.GetStringAt(0), uvBuffers);

	//�@���擾
	FbxArray<FbxVector4> normals;
	FbxMesh->GetPolygonVertexNormals(normals);

	//���_���̎擾
	for (int i = 0; i < polygonVertexCount; i++)
	{
		Vertex vertex;

		//�C���f�b�N�X�o�b�t�@���璸�_�ԍ����擾
		int index = indices[i];

		//���_���W���X�g������W���擾
		vertex.pos.x = vertices[index][0];
		vertex.pos.y = vertices[index][1];
		vertex.pos.z = vertices[index][2];

		//�@�����X�g����@�����擾
		vertex.normal.x = (float)normals[i][0];
		vertex.normal.y = (float)normals[i][1];
		vertex.normal.z = (float)normals[i][2];

		//UV���X�g����擾
		vertex.uv.x = (float)uvBuffers[i][0];
		vertex.uv.y = (float)uvBuffers[i][1];

		//�ۑ����Ă������{�[���̑Ή��\���璸�_�ɑ΂���e���f�[�^���擾
		SetBoneAffectToVertex(vertex, index, BoneTable);

		//���f���̃��b�V���ɒ��_�ǉ�
		ModelMesh.mesh->vertices.emplace_back(vertex);
	}
}

void Importer::LoadFbxIndex(ModelMesh& ModelMesh, FbxMesh* FbxMesh)
{
	//�|���S���̐������A�ԂƂ��ĕۑ�����
	for (int i = 0; i < FbxMesh->GetPolygonCount(); i++)
	{
		//����n�i�E����j
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
	//�}�e���A���̐�
	int materialNum = meshRootNode->GetMaterialCount();

	//���b�V���ɃA�^�b�`�����\��̃}�e���A����
	std::string attachedMatName;
	if (FbxMesh->GetElementMaterialCount() != 0)
	{
		FbxLayerElementMaterial* element = FbxMesh->GetElementMaterial(0);
		//(FBX��ł�)�}�e���A���̃C���f�b�N�X�擾
		int materialIndexNum = element->GetIndexArray().GetAt(0);
		//(FBX��ł�)�}�e���A���擾
		FbxSurfaceMaterial* surface_material = FbxMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(materialIndexNum);
		//���b�V���ɓK�p����}�e���A�����擾
		attachedMatName = surface_material->GetName();
	}

	// �}�e���A�����Ȃ� or ���O�������Ȃ���Ή������Ȃ�
	if (materialNum == 0 || attachedMatName.empty())
	{
		//�f�t�H���g�̃}�e���A������
		ModelMesh.material = std::make_shared<Material>();
		return;
	}

	//�}�e���A���̏����擾
	for (int i = 0; i < materialNum; ++i)
	{
		FbxSurfaceMaterial* material = meshRootNode->GetMaterial(i);

		if (material == nullptr)continue;
		if (material->GetName() != attachedMatName)continue;	//���b�V�����v������}�e���A�����ƈقȂ�΃X���[

		//�}�e���A�����
		std::shared_ptr<Material> newMaterial = std::make_shared<Material>();
		newMaterial->name = material->GetName();

		auto& info = newMaterial->constData;

		//Lambert
		FbxSurfaceLambert* lambert = (FbxSurfaceLambert*)material;

		//�A���r�G���g
		info.ambient.x = (float)lambert->Ambient.Get()[0];
		info.ambient.y = (float)lambert->Ambient.Get()[1];
		info.ambient.z = (float)lambert->Ambient.Get()[2];
		info.ambientFactor = (float)lambert->AmbientFactor.Get();

		//�f�B�t���[�Y
		info.diffuse.x = (float)lambert->Diffuse.Get()[0];
		info.diffuse.y = (float)lambert->Diffuse.Get()[1];
		info.diffuse.z = (float)lambert->Diffuse.Get()[2];
		info.diffuseFactor = (float)lambert->DiffuseFactor.Get();

		//����
		info.emissive.x = (float)lambert->Emissive.Get()[0];
		info.emissive.y = (float)lambert->Emissive.Get()[1];
		info.emissive.z = (float)lambert->Emissive.Get()[2];
		info.emissiveFactor = (float)lambert->EmissiveFactor.Get();

		//���ߓx
		info.transparent = (float)lambert->TransparencyFactor.Get();

		//Phong
		if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

			//�X�y�L�����[
			info.specular.x = (float)phong->Specular.Get()[0];
			info.specular.y = (float)phong->Specular.Get()[1];
			info.specular.z = (float)phong->Specular.Get()[2];
			info.specularFactor = (float)phong->SpecularFactor.Get();

			//����
			info.shininess = (float)phong->Shininess.Get();

			//����
			info.reflection = (float)phong->ReflectionFactor.Get();

			//���˃e�N�X�`��
			FbxFileTexture* emissiveTex = nullptr;
			int emissiveTexNum = phong->Emissive.GetSrcObjectCount();
			if (emissiveTexNum)
			{
				emissiveTex = phong->Emissive.GetSrcObject<FbxFileTexture>(0);
			}

			if (emissiveTex != nullptr)
			{
				auto path = Dir + GetFileName(emissiveTex->GetRelativeFileName());
				//�G�~�b�V�u�}�b�v
				newMaterial->textures[EMISSIVE_TEX].path = path;
				newMaterial->textures[EMISSIVE_TEX].texBuff = D3D12App::Instance()->GenerateTextureBuffer(path);
			}
		}

		//�f�B�q���[�Y���e�N�X�`���̏��������Ă���
		auto prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		//FbxFileTexture���擾
		FbxFileTexture* tex = nullptr;
		int textureNum = prop.GetSrcObjectCount<FbxFileTexture>();
		if (0 < textureNum)
		{
			//prop����FbxFileTexture���擾
			tex = prop.GetSrcObject<FbxFileTexture>(0);
		}
		else
		{
			//���s������}���`�e�N�X�`���̉\�����l����FbxLayeredTexture���w��
			//FbxLayeredTexture����FbxFileTexture���擾
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
	//������
	for (auto& m : models)
	{
		//���ɐ������Ă�����̂�n��
		if (m.first == Dir + FileName)return m.second;
	}

	return std::shared_ptr<Model>();
}

std::shared_ptr<Model> Importer::LoadHSMModel(const std::string& Dir, const std::string& FileName, const std::string& Path)
{
	static const std::string FUNC_NAME = "LoadHSMModel";
	static const std::string MSG_TAIL = "�̓ǂݎ��Ɏ��s\n";
	std::shared_ptr<Model>model = std::make_shared<Model>(Dir, FileName);

	FILE* fp;
	fopen_s(&fp, Path.c_str(), "rb");
	ErrorMessage(FUNC_NAME, fp == nullptr, ".hsm�t�@�C���̃o�C�i���ǂݎ�胂�[�h�ł̃I�[�v���Ɏ��s\n");

	//�ŏI�X�V�����i�f�[�^�i�[�͂��Ȃ��j
	FILETIME dummy;
	ErrorMessage(FUNC_NAME, !LoadData(fp, &dummy, sizeof(dummy), 1), "�ŏI�X�V����(�_�~�[)" + MSG_TAIL);

	size_t size;

	//���b�V���̐�
	unsigned short meshNum;
	ErrorMessage(FUNC_NAME, !LoadData(fp, &meshNum, sizeof(meshNum), 1), "���b�V���̐�" + MSG_TAIL);

	//���b�V������
	model->meshes.resize(meshNum);

	//���b�V�����
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		//���b�V���擾
		auto& mesh = model->meshes[meshIdx];
		mesh.mesh = std::make_shared<Mesh<ModelMesh::Vertex_Model>>();
		mesh.material = std::make_shared<Material>();

		//���b�V�����T�C�Y
		ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "���b�V�����T�C�Y" + MSG_TAIL);
		//���b�V����
		mesh.mesh->name.resize(size);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &mesh.mesh->name[0], size, 1), "���b�V����" + MSG_TAIL);

		//���_��
		unsigned short vertNum;
		ErrorMessage(FUNC_NAME, !LoadData(fp, &vertNum, sizeof(vertNum), 1), "���_��" + MSG_TAIL);
		//���_���
		mesh.mesh->vertices.resize(vertNum);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &mesh.mesh->vertices[0], sizeof(ModelMesh::Vertex_Model), vertNum), "���_���" + MSG_TAIL);

		//�C���f�b�N�X��
		unsigned short idxNum;
		ErrorMessage(FUNC_NAME, !LoadData(fp, &idxNum, sizeof(idxNum), 1), "�C���f�b�N�X��" + MSG_TAIL);
		//�C���f�b�N�X���
		mesh.mesh->indices.resize(idxNum);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &mesh.mesh->indices[0], sizeof(unsigned int), idxNum), "�C���f�b�N�X���" + MSG_TAIL);

		//�}�e���A���擾
		auto& material = mesh.material;
		//�}�e���A�����T�C�Y
		ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "�}�e���A�����T�C�Y" + MSG_TAIL);
		//�}�e���A����
		material->name.resize(size);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &material->name[0], size, 1), "�}�e���A����" + MSG_TAIL);
		//�}�e���A�����
		ErrorMessage(FUNC_NAME, !LoadData(fp, &material->constData, sizeof(Material::ConstData), 1), "�}�e���A�����" + MSG_TAIL);

		//�摜�t�@�C���e�N�X�`���ێ��t���O
		unsigned char haveTex[MATERIAL_TEX_TYPE_NUM];
		ErrorMessage(FUNC_NAME, !LoadData(fp, haveTex, sizeof(unsigned char), MATERIAL_TEX_TYPE_NUM), "�摜�t�@�C���e�N�X�`���ێ��t���O" + MSG_TAIL);
		//�摜�t�@�C���e�N�X�`���p�X���
		for (int texIdx = 0; texIdx < MATERIAL_TEX_TYPE_NUM; ++texIdx)
		{
			if (!haveTex[texIdx])continue;

			//�t�@�C���p�X�T�C�Y
			ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "�t�@�C���p�X�T�C�Y" + MSG_TAIL);
			//�t�@�C���p�X
			material->textures[texIdx].path.resize(size);
			ErrorMessage(FUNC_NAME, !LoadData(fp, &material->textures[texIdx].path[0], size, 1), "�摜�t�@�C���p�X" + MSG_TAIL);
		}

		mesh.material->CreateBuff();
		mesh.mesh->CreateBuff();
	}

	//�X�P���g���擾
	auto& skel = model->skelton;
	//�{�[���̐�
	unsigned short boneNum;
	ErrorMessage(FUNC_NAME, !LoadData(fp, &boneNum, sizeof(boneNum), 1), "�{�[����" + MSG_TAIL);
	skel.bones.resize(boneNum);

	//�{�[�����
	for (int boneIdx = 0; boneIdx < boneNum; ++boneIdx)
	{
		//�{�[���擾
		auto& bone = skel.bones[boneIdx];
		//�{�[�����T�C�Y
		ErrorMessage(FUNC_NAME, !LoadData(fp, &size, sizeof(size), 1), "�{�[�����T�C�Y" + MSG_TAIL);
		//�{�[����
		bone.name.resize(size);
		ErrorMessage(FUNC_NAME, !LoadData(fp, &bone.name[0], size, 1), "�{�[����" + MSG_TAIL);

		//���O�ȊO�̃{�[�����
		ErrorMessage(FUNC_NAME, !LoadData(fp, &bone.parent, Bone::GetSizeWithOutName(), 1), "�{�[�����" + MSG_TAIL);
	}
	skel.CreateBoneTree();

	fclose(fp);

	RegisterImportModel(Dir, FileName, model);

	return model;
}

void Importer::SaveHSMModel(const std::string& FileNameTail, std::shared_ptr<Model> Model, const FILETIME& LastWriteTime)
{
	static const std::string FUNC_NAME = "SaveHSMModel";
	static const std::string MSG_TAIL = "�̏������݂Ɏ��s\n";

	FILE* fp;
	fopen_s(&fp, (Model->header.dir + Model->header.GetModelName() + FileNameTail + ".hsm").c_str(), "wb");
	ErrorMessage(FUNC_NAME, fp == nullptr, ".hsm�t�@�C���̃o�C�i���������݃��[�h�ł̃I�[�v���Ɏ��s\n");

	//�ŏI�X�V����
	ErrorMessage(FUNC_NAME, !SaveData(fp, &LastWriteTime, sizeof(LastWriteTime), 1), "�ŏI�X�V����" + MSG_TAIL);

	size_t size;

	//���b�V���̐�
	unsigned short meshNum = Model->meshes.size();
	ErrorMessage(FUNC_NAME, !SaveData(fp, &meshNum, sizeof(meshNum), 1), "���b�V���̐�" + MSG_TAIL);

	//���b�V�����
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		//���b�V���擾
		auto& mesh = Model->meshes[meshIdx];
		//���b�V�����T�C�Y
		size = mesh.mesh->name.length();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "���b�V�����T�C�Y" + MSG_TAIL);
		//���b�V����
		ErrorMessage(FUNC_NAME, !SaveData(fp, mesh.mesh->name.data(), size, 1), "���b�V����" + MSG_TAIL);

		//���_��
		unsigned short vertNum = mesh.mesh->vertices.size();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &vertNum, sizeof(vertNum), 1), "���_��" + MSG_TAIL);
		//���_���
		ErrorMessage(FUNC_NAME, !SaveData(fp, mesh.mesh->vertices.data(), sizeof(ModelMesh::Vertex_Model), vertNum), "���_���" + MSG_TAIL);

		//�C���f�b�N�X��
		unsigned short idxNum = mesh.mesh->indices.size();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &idxNum, sizeof(idxNum), 1), "�C���f�b�N�X��" + MSG_TAIL);
		//�C���f�b�N�X���
		ErrorMessage(FUNC_NAME, !SaveData(fp, mesh.mesh->indices.data(), sizeof(unsigned int), idxNum), "�C���f�b�N�X���" + MSG_TAIL);

		//�}�e���A���擾
		auto& material = mesh.material;
		//�}�e���A�����T�C�Y
		size = material->name.length();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "�}�e���A�����T�C�Y" + MSG_TAIL);
		//�}�e���A����
		ErrorMessage(FUNC_NAME, !SaveData(fp, material->name.data(), size, 1), "�}�e���A����" + MSG_TAIL);
		//�}�e���A�����
		ErrorMessage(FUNC_NAME, !SaveData(fp, &material->constData, sizeof(Material::ConstData), 1), "�}�e���A�����" + MSG_TAIL);

		//�摜�t�@�C���e�N�X�`���ێ��t���O
		unsigned char haveTex[MATERIAL_TEX_TYPE_NUM] = { 0 };
		for (int texIdx = 0; texIdx < MATERIAL_TEX_TYPE_NUM; ++texIdx)
		{
			if (material->textures[texIdx].path.empty())continue;

			//�e�N�X�`�������蓖�Ă��Ă���
			haveTex[texIdx] = 1;
		}
		ErrorMessage(FUNC_NAME, !SaveData(fp, haveTex, sizeof(unsigned char), MATERIAL_TEX_TYPE_NUM), "�}�e���A���̃e�N�X�`���ێ��t���O" + MSG_TAIL);

		//�摜�t�@�C���e�N�X�`���p�X���
		for(int texIdx = 0;texIdx < MATERIAL_TEX_TYPE_NUM;++texIdx)
		{
			if (!haveTex[texIdx])continue;

			//�摜�t�@�C���p�X�T�C�Y
			size = material->textures[texIdx].path.length();
			ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "�e�N�X�`���摜�t�@�C���p�X�T�C�Y" + MSG_TAIL);
			ErrorMessage(FUNC_NAME, !SaveData(fp, material->textures[texIdx].path.data(), size, 1), "�e�N�X�`���摜�t�@�C���p�X" + MSG_TAIL);
		}
	}

	//�X�P���g���擾
	auto& skel = Model->skelton;
	//�{�[���̐�
	unsigned short boneNum = skel.bones.size();
	ErrorMessage(FUNC_NAME, !SaveData(fp, &boneNum, sizeof(boneNum), 1), "�{�[����" + MSG_TAIL);
	//�{�[�����
	for (int boneIdx = 0; boneIdx < boneNum; ++boneIdx)
	{
		//�{�[���擾
		auto& bone = skel.bones[boneIdx];
		//�{�[�����T�C�Y
		size = bone.name.length();
		ErrorMessage(FUNC_NAME, !SaveData(fp, &size, sizeof(size), 1), "�{�[�����T�C�Y" + MSG_TAIL);
		//�{�[����
		ErrorMessage(FUNC_NAME, !SaveData(fp, bone.name.data(), size, 1), "�{�[����" + MSG_TAIL);
		//�{�[�����
		ErrorMessage(FUNC_NAME, !SaveData(fp, &bone.parent, Bone::GetSizeWithOutName(), 1), "�{�[�����" + MSG_TAIL);
	}

	fclose(fp);
}

Importer::Importer()
{
	static const std::string FUNC_NAME = "�R���X�g���N�^";

	//�}�l�[�W������
	fbxManager = FbxManager::Create();
	ErrorMessage(FUNC_NAME, fbxManager == nullptr, "FBX�}�l�[�W�������Ɏ��s\n");

	//IOSetting�𐶐�
	ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	ErrorMessage(FUNC_NAME, ioSettings == nullptr, "FBXIOSetting�����Ɏ��s\n");

	//�C���|�[�^����
	fbxImporter = FbxImporter::Create(fbxManager, "");
	ErrorMessage(FUNC_NAME, fbxImporter == nullptr, "FBX�C���|�[�^�����Ɏ��s\n");
}

std::shared_ptr<Model> Importer::LoadFBXModel(const std::string& Dir, const std::string& FileName)
{
	static const std::string FUNC_NAME = "LoadFBX";
	static const std::string HSM_TAIL = "_fbx";

	std::shared_ptr<Model>result;

	//���ɐ������Ă��邩
	result = CheckAlreadyExsit(Dir, FileName);
	if (result)return result;	//�������Ă����炻���Ԃ�

	//�g���q�擾
	const auto ext = "." + KuroFunc::GetExtension(FileName);
	ErrorMessage(FUNC_NAME, ext != ".fbx", "�g���q�������܂���\n");

	//���f�����擾(�t�@�C��������g���q������������)
	auto modelName = FileName;
	modelName.erase(modelName.size() - ext.size());

	//�t�@�C���p�X
	const auto path = Dir + FileName;

	//fbx�t�@�C���̍ŏI�X�V������ǂݎ��
	FILETIME fbxLastWriteTime;
	HANDLE fbxFile = CreateFile(
		KuroFunc::GetWideStrFromStr(path).c_str(),
		0,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	ErrorMessage(FUNC_NAME, !GetFileTime(fbxFile, NULL, NULL, &fbxLastWriteTime), "fbx�t�@�C���̍ŏI�X�V�����ǂݎ��Ɏ��s\n");

	//hsm�t�@�C�������݂��邩
	const auto hsmPath = Dir + modelName + HSM_TAIL + ".hsm";
	if (canLoadHSM && KuroFunc::ExistFile(hsmPath))
	{
		//hsm�t�@�C���ɋL�^���ꂽ���f���̍ŏI�X�V�����f�[�^��ǂݎ��
		FILETIME modelLastWriteTime;
		FILE* fp;
		fopen_s(&fp, hsmPath.c_str(), "rb");
		if (fp != NULL)
		{
			ErrorMessage(FUNC_NAME, fread(&modelLastWriteTime, sizeof(FILETIME), 1, fp) < 1, "hsm�t�@�C���̍ŏI�X�V�����ǂݎ��Ɏ��s\n");
			fclose(fp);
		}

		//fbx�t�@�C���̍ŏI�X�V�����Ɣ�r�A�����Ȃ�hsm�t�@�C�������[�h���Ă����Ԃ�
		if (modelLastWriteTime.dwHighDateTime == fbxLastWriteTime.dwHighDateTime
			&& modelLastWriteTime.dwLowDateTime == fbxLastWriteTime.dwLowDateTime)
		{
			return LoadHSMModel(Dir, FileName, hsmPath);
		}
	}


	//�t�@�C���̑��݂��m�F
	ErrorMessage(FUNC_NAME, !KuroFunc::ExistFile(path), "�t�@�C�������݂��܂���\n");

	//�t�@�C��������������
	ErrorMessage(FUNC_NAME, fbxImporter->Initialize(path.c_str(), -1, fbxManager->GetIOSettings()) == false, "�t�@�C���̏������Ɏ��s\n");

	//�V�[���I�u�W�F�N�g����
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
	ErrorMessage(FUNC_NAME, fbxScene == nullptr, "FBX�V�[�������Ɏ��s\n");

	//�V�[���I�u�W�F�N�g��fbx�t�@�C�����̏��𗬂�����
	ErrorMessage(FUNC_NAME, fbxImporter->Import(fbxScene) == false, "FBX�V�[���ւ�FBX�t�@�C����񗬂����݂Ɏ��s\n");

	//�V�[�����̃m�[�h�̃|���S����S�ĎO�p�`�ɂ���
	FbxGeometryConverter converter(fbxManager);

	//�SFbxMesh���}�e���A���P�ʂŕ���
	converter.SplitMeshesPerMaterial(fbxScene, true);

	//�|���S�����O�p�`�ɂ���
	converter.Triangulate(fbxScene, true);

	//���b�V�����ǂݎ��A�ۑ�
	std::vector<FbxMesh*>fbxMeshes;
	TraceFbxMesh(fbxScene->GetRootNode(), &fbxMeshes);

	result = std::make_shared<Model>(Dir, FileName);
	Skeleton skel;

	for (int i = 0; i < fbxMeshes.size(); ++i)
	{
		auto& fbxMesh = fbxMeshes[i];

		//���f���p���b�V������
		ModelMesh mesh;
		mesh.mesh = std::make_shared<Mesh<ModelMesh::Vertex_Model>>();

		//���b�V�����擾
		const std::string meshName = fbxMesh->GetName();

		//���b�V�����Z�b�g
		mesh.mesh->name = meshName;

		//�{�[�������_�ɗ^����e���Ɋւ�����e�[�u��
		BoneTable boneAffectTable;

		//�{�[���i�X�P���g���j
		LoadFbxSkin(skel, fbxMesh, boneAffectTable);
		//���_
		LoadFbxVertex(mesh, fbxMesh, boneAffectTable);
		//�C���f�b�N�X
		LoadFbxIndex(mesh, fbxMesh);
		//�}�e���A��
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