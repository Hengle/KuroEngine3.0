#include "Skeleton.h"
#include"KuroFunc.h"

void Skeleton::CreateBoneTree()
{
	//�{�[�����Ȃ��Ȃ疳��
	if (bones.empty())return;

	//�C���f�b�N�X�Ɩ��O�̑Ή��֌W�\�z�̂��߂Ɍ�Ŏg��
	std::vector<std::string>boneNames(bones.size());

	//�{�[���m�[�h�}�b�v�����
	for (int idx = 0; idx < bones.size(); ++idx)
	{
		auto& bone = bones[idx];
		boneNames[idx] = bone.name;
		auto& node = boneNodeTable[bone.name];
		node.boneIdx = idx;
		node.startPos = bone.pos;
	}

	//�e�q�֌W�\�z
	for (int i = 0; i < bones.size(); ++i)
	{
		auto& bone = bones[i];
		//�e�C���f�b�N�X���`�F�b�N(���肦�Ȃ��ԍ��Ȃ�Ƃ΂�)
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
	KuroFunc::ErrorMessage(bones.empty(), "Skeleton", "GetIndex", "�{�[����񂪂���܂���\n");
	KuroFunc::ErrorMessage(boneNodeTable[BoneName].boneIdx == -1, "Skeleton", "GetIndex", "���݂��Ȃ��{�[�����Q�Ƃ���܂��� (" + BoneName + ")\n");
	return boneNodeTable[BoneName].boneIdx;
}
