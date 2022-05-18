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
		vertex.pos.x = -vertices[index][0];
		vertex.pos.y = vertices[index][1];
		vertex.pos.z = vertices[index][2];

		//�@�����X�g����@�����擾
		vertex.normal.x = -(float)normals[i][0];
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
		ModelMesh.mesh->indices.emplace_back(i * 3);
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
		info.lambert.ambient.x = (float)lambert->Ambient.Get()[0];
		info.lambert.ambient.y = (float)lambert->Ambient.Get()[1];
		info.lambert.ambient.z = (float)lambert->Ambient.Get()[2];
		info.lambert.ambientFactor = (float)lambert->AmbientFactor.Get();

		//�f�B�t���[�Y
		info.lambert.diffuse.x = (float)lambert->Diffuse.Get()[0];
		info.lambert.diffuse.y = (float)lambert->Diffuse.Get()[1];
		info.lambert.diffuse.z = (float)lambert->Diffuse.Get()[2];
		info.lambert.diffuseFactor = (float)lambert->DiffuseFactor.Get();

		//����
		info.lambert.emissive.x = (float)lambert->Emissive.Get()[0];
		info.lambert.emissive.y = (float)lambert->Emissive.Get()[1];
		info.lambert.emissive.z = (float)lambert->Emissive.Get()[2];
		info.lambert.emissiveFactor = (float)lambert->EmissiveFactor.Get();

		//���ߓx
		info.lambert.transparent = (float)lambert->TransparencyFactor.Get();

		//Phong
		if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

			//�X�y�L�����[
			info.phong.specular.x = (float)phong->Specular.Get()[0];
			info.phong.specular.y = (float)phong->Specular.Get()[1];
			info.phong.specular.z = (float)phong->Specular.Get()[2];
			info.phong.specularFactor = (float)phong->SpecularFactor.Get();

			//����
			info.phong.shininess = (float)phong->Shininess.Get();

			//����
			info.phong.reflection = (float)phong->ReflectionFactor.Get();

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
				newMaterial->texBuff[EMISSIVE_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
			}
		}

		//PBR
		//�x�[�X�J���[
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
		//�����x
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
		//PBR�X�y�L�����[
		const auto propSpecular = FbxSurfaceMaterialUtils::GetProperty("specular", material);
		if (propSpecular.IsValid())
		{
			newMaterial->constData.pbr.specular = propSpecular.Get<float>();
		}
		//�e��
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

		//�f�B�q���[�Y���e�N�X�`���̏��������Ă���
		{
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
				const auto fileName = GetFileName(tex->GetRelativeFileName());
				if (!fileName.empty())
				{
					auto path = Dir + fileName;
					newMaterial->texBuff[COLOR_TEX] = D3D12App::Instance()->GenerateTextureBuffer(path);
				}
			}
		}

		//�@���}�b�v
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

	//�{�[���̃A�j���[�V���������ǂݍ��܂Ȃ�
	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		std::string boneName = FbxNode->GetName();

		//�{�[���P�ʃA�j���[�V�����擾
		auto& boneAnim = ModelAnimation.boneAnim[boneName];

		FbxAnimCurve* animCurve;

		//���W
		animCurve = FbxNode->LclTranslation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animCurve)LoadAnimCurve(animCurve, boneAnim.pos[Skeleton::BoneAnimation::X]);

		animCurve = FbxNode->LclTranslation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.pos[Skeleton::BoneAnimation::Y]);

		animCurve = FbxNode->LclTranslation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.pos[Skeleton::BoneAnimation::Z]);

		//��]
		animCurve = FbxNode->LclRotation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animCurve)LoadAnimCurve(animCurve, boneAnim.rotate[Skeleton::BoneAnimation::X]);

		animCurve = FbxNode->LclRotation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.rotate[Skeleton::BoneAnimation::Y]);

		animCurve = FbxNode->LclRotation.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.rotate[Skeleton::BoneAnimation::Z]);

		//�X�P�[��
		animCurve = FbxNode->LclScaling.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animCurve)LoadAnimCurve(animCurve, boneAnim.scale[Skeleton::BoneAnimation::X]);

		animCurve = FbxNode->LclScaling.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.scale[Skeleton::BoneAnimation::Y]);

		animCurve = FbxNode->LclScaling.GetCurve(FbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animCurve)	LoadAnimCurve(animCurve, boneAnim.scale[Skeleton::BoneAnimation::Z]);
	}

	// �q�m�[�h�ɑ΂��čċA�Ăяo��
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

	// ���_�ʒu���A�N�Z�b�T�̎擾
	auto& idPos = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_POSITION);
	auto& accPos = Doc.accessors.Get(idPos);
	auto vertPos = Reader.ReadBinaryData<float>(Doc, accPos);
	// �@�����A�N�Z�b�T�̎擾
	auto& idNrm = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_NORMAL);
	auto& accNrm = Doc.accessors.Get(idNrm);
	auto vertNrm = Reader.ReadBinaryData<float>(Doc, accNrm);
	// �e�N�X�`�����W���A�N�Z�b�T�̎擾
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
		// ���_�f�[�^�̍\�z
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

	//�^���W�F���g���A�N�Z�b�T�擾
	if (GLTFPrimitive.HasAttribute(ACCESSOR_TANGENT))
	{
		auto& idTangent = GLTFPrimitive.GetAttributeAccessorId(ACCESSOR_TANGENT);
		auto& accTangent = Doc.accessors.Get(idTangent);
		auto vertTangent = Reader.ReadBinaryData<float>(Doc, accTangent);
		for (uint32_t i = 0; i < vertexCount; ++i)
		{
			int vid0 = 3 * i, vid1 = 3 * i + 1, vid2 = 3 * i + 2;
			ModelMesh.mesh->vertices[i].tangent = { vertTangent[vid0],vertTangent[vid1],vertTangent[vid2] };
			ModelMesh.mesh->vertices[i].binormal = ModelMesh.mesh->vertices[i].normal.Cross(ModelMesh.mesh->vertices[i].tangent);
		}
	}

	// ���_�C���f�b�N�X�p�A�N�Z�b�T�̎擾
	auto& idIndex = GLTFPrimitive.indicesAccessorId;
	auto& accIndex = Doc.accessors.Get(idIndex);

	// �C���f�b�N�X�f�[�^
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
	//������
	for (auto& m : models)
	{
		//���ɐ������Ă�����̂�n��
		if (m.first == Dir + FileName)return m.second;
	}

	return std::shared_ptr<Model>();
}

/*
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
*/

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
	printf("glTF���[�h\nDir : %s , FileName : %s\n", Dir.c_str(), FileName.c_str());

	static const std::string FUNC_NAME = "LoadFBX";
	static const std::string HSM_TAIL = "_fbx";

	std::shared_ptr<Model>result;

	//���ɐ������Ă��邩
	result = CheckAlreadyExsit(Dir, FileName);
	if (result)return result;	//�������Ă����炻���Ԃ�

	//�g���q�擾
	const auto ext = "." + KuroFunc::GetExtension(FileName);
	ErrorMessage(FUNC_NAME, ext != ".fbx" && ext != ".FBX", "�g���q�������܂���\n");

	//���f�����擾(�t�@�C��������g���q������������)
	auto modelName = FileName;
	modelName.erase(modelName.size() - ext.size());

	//�t�@�C���p�X
	const auto path = Dir + FileName;

	/*
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
	*/

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

	//�A�j���[�V�����̐�
	int animStackCount = fbxImporter->GetAnimStackCount();
	for (int animIdx = 0; animIdx < animStackCount; ++animIdx)
	{
		Skeleton::ModelAnimation animation;

		// AnimStack�ǂݍ��݁iAnimLayer�̏W��)
		FbxAnimStack* animStack = fbxScene->GetSrcObject<FbxAnimStack>(animIdx);
		animation.name = animStack->GetName();	//�A�j���[�V�������擾

		FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
		TraceBoneAnim(animation, fbxScene->GetRootNode(), animLayer);

		skel.animations.emplace_back(animation);

		////�A�j���[�V���������蓖�Ă��Ă���h���ʁh�̐��iAnimCurve �̏W���j
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

void Importer::LoadGLTFMaterial(const MATERIAL_TEX_TYPE& Type, std::weak_ptr<Material> AttachMaterial, const Microsoft::glTF::Image& Img, const std::string& Dir, const Microsoft::glTF::GLTFResourceReader& Reader, const Microsoft::glTF::Document& Doc)
{
	auto material = AttachMaterial.lock();
	//�e�N�X�`���摜�t�@�C���ǂݍ���
	if (!Img.uri.empty())
	{
		std::string path = Dir + Img.uri;
		material->texBuff[Type] = D3D12App::Instance()->GenerateTextureBuffer(path);
	}
	//�e�N�X�`���摜��gltf�ɖ��ߍ��܂�Ă���
	else if (!Img.bufferViewId.empty())
	{
		auto imageBufferView = Doc.bufferViews.Get(Img.bufferViewId);
		auto imageData = Reader.ReadBinaryData<char>(Doc, imageBufferView);
		std::string path = "glTF - Load (" + Img.mimeType + ") - " + Img.name;
		material->texBuff[Type] = D3D12App::Instance()->GenerateTextureBuffer(imageData);
	}
}

#include<sstream>
std::shared_ptr<Model> Importer::LoadGLTFModel(const std::string& Dir, const std::string& FileName)
{
	printf("glTF���[�h\nDir : %s , FileName : %s\n", Dir.c_str(), FileName.c_str());
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

	//�g���q ".gltf" ".glb" �̔���
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

	KuroFunc::ErrorMessage(!resourceReader, "Importer", "LoadGLTFModel", "�t�@�C���̊g���q�� glb �ł� gltf �ł�����܂���\n");

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

	//�X�P���g���ǂݍ��ݓr��
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

	//�}�e���A���ǂݍ���
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

		//�J���[�e�N�X�`��
		auto textureId = m.metallicRoughness.baseColorTexture.textureId;
		if (!textureId.empty())
		{
			auto& texture = doc.textures.Get(textureId);
			auto& image = doc.images.Get(texture.imageId);
			LoadGLTFMaterial(COLOR_TEX, material, image, Dir, *resourceReader, doc);
		}
		//�G�~�b�V�u
		textureId = m.emissiveTexture.textureId;
		if (!textureId.empty())
		{
			auto& texture = doc.textures.Get(textureId);
			auto& image = doc.images.Get(texture.imageId);
			LoadGLTFMaterial(EMISSIVE_TEX, material, image, Dir, *resourceReader, doc);
		}
		//�m�[�}���}�b�v
		textureId = m.normalTexture.textureId;
		if (!textureId.empty())
		{
			auto& texture = doc.textures.Get(textureId);
			auto& image = doc.images.Get(texture.imageId);
			LoadGLTFMaterial(NORMAL_TEX, material, image, Dir, *resourceReader, doc);
		}
		//���t�l�X
		textureId = m.metallicRoughness.metallicRoughnessTexture.textureId;
		if (!textureId.empty())
		{
			auto& texture = doc.textures.Get(textureId);
			auto& image = doc.images.Get(texture.imageId);
			LoadGLTFMaterial(ROUGHNESS_TEX, material, image, Dir, *resourceReader, doc);
		}

		material->CreateBuff();
		loadMaterials.emplace_back(material);
	}

	for (const auto& glTFMesh : doc.meshes.Elements())
	{
		for (const auto& meshPrimitive : glTFMesh.primitives)
		{
			//���f���p���b�V������
			ModelMesh mesh;
			mesh.mesh = std::make_shared<Mesh<ModelMesh::Vertex_Model>>();

			//���_ & �C���f�b�N�X���
			LoadGLTFPrimitive(mesh, meshPrimitive, *resourceReader, doc);

			if (doc.materials.Has(meshPrimitive.materialId))
			{
				int materialIdx = int(doc.materials.GetIndex(meshPrimitive.materialId));
				mesh.material = loadMaterials[materialIdx];
			}
			else
			{
				mesh.material = std::make_shared<Material>();
			}

			mesh.mesh->CreateBuff();
			result->meshes.emplace_back(mesh);
		}
	}

	return result;
}
