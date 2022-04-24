#pragma once
#include<string>
#include"Vec.h"
#include"KuroMath.h"
#include<map>
#include"Animation.h"
#include<array>

class Bone
{
public:
	static size_t GetSizeWithOutName()
	{
		return sizeof(char) + sizeof(int) + sizeof(Vec3<float>) + sizeof(Matrix);
	}

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
	struct BoneAnimation
	{
		static const enum { X, Y, Z, NUM };
		std::array<Animation, NUM>pos;
		std::array<Animation, NUM>rotate;
		std::array < Animation, NUM>scale;
	};
	struct ModelAnimation
	{
		std::string name;	//�A�j���[�V������
		std::map<std::string, BoneAnimation>boneAnim;	//�{�[���P�ʂ̃A�j���[�V����
	};

	std::vector<Bone>bones;
	std::map<std::string, BoneNode>boneNodeTable;
	std::vector<ModelAnimation>animations;	//�A�j���[�V�������iSkeleton���A�j���[�V�������s����ł͖����BAnimator����̎Q�Ɨp�j
	void CreateBoneTree();
	int GetIndex(const std::string& BoneName);
};

