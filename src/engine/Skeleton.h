#pragma once
#include<string>
#include"Vec.h"
#include"KuroMath.h"
#include<map>

class Bone
{
public:
	std::string name;
	char parent = -1;	//�e�{�[��
	int transLayer = 0;	//�ό`�K�w
	Vec3<float> pos = { 0.0f,0.0f,0.0f };
	Matrix offsetMat = DirectX::XMMatrixIdentity();
};

class BoneNode
{
public:
	int boneIdx = -1;	//�{�[���C���f�b�N�X
	Vec3<float>startPos;//�{�[����_�i��]�̒��S�j
	Vec3<float>endPos;	//�{�[����[�_�i���ۂ̃X�L�j���O�ɂ͗��p���Ȃ��j
	std::vector<BoneNode*>children;	//�q�m�[�h

	operator bool() { return boneIdx != -1; }
};

class Skeleton
{
public:
	std::vector<Bone>bones;
	std::map<std::string, BoneNode>boneNodeTable;
	void CreateBoneTree();
	int GetIndex(const std::string& BoneName);
};

