#pragma once
#include"Singleton.h"
#include<string>
#include<fbxsdk.h>
#include<vector>
#include<memory>
#include<map>
#include<forward_list>
#include"ModelMesh.h"
#include"Skeleton.h"
class Model;

class Importer : public Singleton<Importer>
{
	//���f����p���_
	using Vertex = ModelMesh::Vertex_Model;

	//�G���[���b�Z�[�W�\��
	void ErrorMessage(const std::string& FuncName, const bool& Fail, const std::string& Comment, void (Importer::*Func)() = nullptr);
	//HSM���[�h�ŗp����f�[�^�̓ǂݎ��֐��i���������true��Ԃ�)
	bool LoadData(FILE* Fp, void* Data, const size_t& Size, const int& ElementNum);
	//HSM�Z�[�u�ŗp����f�[�^�̏������݊֐��i���������true��Ԃ�)
	bool SaveData(FILE* Fp, const void* Data, const size_t& Size, const int& ElementNum);


#pragma region FBX�֘A
	struct FbxBoneAffect	//�{�[���ƒ��_�̊֘A���L�^���邽�߂̃N���X
	{
		signed short index;
		float weight;
	};
	//�{�[�������_�ɗ^����e���Ɋւ�����e�[�u��
	//< ���_�C���f�b�N�X�A���(�ϒ��A�S�܂�)>
	//�T�ȏ�ǂݍ��܂�Ă����ꍇ�A���_�ǂݍ��ݎ��ɖ��������
	using BoneTable = std::map<int, std::forward_list<FbxBoneAffect>>;

	FbxManager* fbxManager = nullptr;
	FbxIOSettings* ioSettings = nullptr;
	FbxImporter* fbxImporter = nullptr;

	//FBX�֘A�f�o�C�X�̍폜����
	void FbxDeviceDestroy();
	//FbxMesh��H���Ĕz��Ƃ��ĕۑ�
	void TraceFbxMesh(FbxNode* Node, std::vector<FbxMesh*>* Mesh);
	//FbxMatrix��XMMATRIX�ɕϊ�
	XMMATRIX FbxMatrixConvert(const FbxMatrix& Mat);	
	//�e�탂�f�����ǂݎ��
	void LoadFbxBone(FbxCluster* Cluster, const int& BoneIndex, BoneTable& BoneTable);
	void LoadFbxSkin(Skeleton& Skel, FbxMesh* FbxMesh, BoneTable& BoneTable);
	void SetBoneAffectToVertex(Vertex& Vertex, const int& VertexIdx, BoneTable& BoneTable);	//���_�ɑΉ��{�[�����i�[
	void LoadFbxVertex(ModelMesh& ModelMesh, FbxMesh* FbxMesh, BoneTable& BoneTable);
	void LoadFbxIndex(ModelMesh& ModelMesh, FbxMesh* FbxMesh);
	std::string GetFileName(std::string FbxRelativeFileName);
	void LoadFbxMaterial(const std::string& Dir, ModelMesh& ModelMesh, FbxMesh* FbxMesh);
#pragma endregion

	//�C���|�[�g�������f��
	std::map<std::string, std::shared_ptr<Model>>models;
	std::shared_ptr<Model> CheckAlreadyExsit(const std::string& Dir, const std::string& FileName);

	//HSM�̓G���W����p�̑��탂�f���Ƃ̒�����i���̃t�H�[�}�b�g�ǂݍ��݂͎��ԃR�X�g�������H�j
	std::shared_ptr<Model>LoadHSMModel(const std::string& Path);
	void SaveHSMModel(const std::string& FileNameTail, std::shared_ptr<Model>Model, const FILETIME& LastWriteTime);

	//HSM�����݂���ꍇ��HSM����ǂݍ��߂�悤�ɂ��邩(HSM�̃t�H�[�}�b�g���ς�����Ƃ��̓t���O���I�t�ɂ���)
	const bool canLoadHSM = true;

	Importer();
public:
	~Importer() { FbxDeviceDestroy(); }
	std::shared_ptr<Model> LoadFBXModel(const std::string& Dir, const std::string& FileName);
};
