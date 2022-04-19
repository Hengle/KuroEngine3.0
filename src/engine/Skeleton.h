#pragma once
#include<string>
#include"Vec.h"
#include"KuroMath.h"
#include<map>

class Bone
{
public:
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
	std::vector<Bone>bones;
	std::map<std::string, BoneNode>boneNodeTable;
	void CreateBoneTree();
	int GetIndex(const std::string& BoneName);
};

