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
	char parent = -1;	//親ボーン
	int transLayer = 0;	//変形階層
	Vec3<float> pos = { 0.0f,0.0f,0.0f };
	Matrix offsetMat = DirectX::XMMatrixIdentity();
};

class BoneNode
{
public:
	int boneIdx = -1;	//ボーンインデックス
	Vec3<float>startPos;//ボーン基準点（回転の中心）
	Vec3<float>endPos;	//ボーン先端点（実際のスキニングには利用しない）
	std::vector<BoneNode*>children;	//子ノード

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
		std::string name;	//アニメーション名
		std::map<std::string, BoneAnimation>boneAnim;	//ボーン単位のアニメーション
	};

	std::vector<Bone>bones;
	std::map<std::string, BoneNode>boneNodeTable;
	std::vector<ModelAnimation>animations;	//アニメーション情報（Skeletonがアニメーションを行う訳では無い。Animatorからの参照用）
	void CreateBoneTree();
	int GetIndex(const std::string& BoneName);
};

