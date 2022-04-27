#include "Skeleton.h"
#include"KuroFunc.h"

void Skeleton::CreateBoneTree()
{
	//ボーンがないなら無視
	if (bones.empty())return;

	//インデックスと名前の対応関係構築のために後で使う
	std::vector<std::string>boneNames(bones.size());

	//ボーンノードマップを作る
	for (int idx = 0; idx < bones.size(); ++idx)
	{
		auto& bone = bones[idx];
		boneNames[idx] = bone.name;
		auto& node = boneNodeTable[bone.name];
		node.boneIdx = idx;
		node.startPos = bone.pos;
	}

	//親子関係構築
	for (int i = 0; i < bones.size(); ++i)
	{
		auto& bone = bones[i];
		//親インデックスをチェック(ありえない番号ならとばす)
		if (bone.parent < 0 || bones.size() < bone.parent)
		{
			continue;
		}
		auto parentName = boneNames[bone.parent];
		boneNodeTable[parentName].children.emplace_back(&boneNodeTable[bone.name]);
	}
}

int Skeleton::GetIndex(const std::string& BoneName)
{
	KuroFunc::ErrorMessage(bones.empty(), "Skeleton", "GetIndex", "ボーン情報がありません\n");
	KuroFunc::ErrorMessage(boneNodeTable[BoneName].boneIdx == -1, "Skeleton", "GetIndex", "存在しないボーンが参照されました (" + BoneName + ")\n");
	return boneNodeTable[BoneName].boneIdx;
}
