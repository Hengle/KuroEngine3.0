#pragma once
#include<string>
#include<vector>
#include"ImportHeader.h"
#include"Skeleton.h"
#include"ModelMesh.h"

struct Model
{
public:
	//�w�b�_�i���f�����j
	ImportHeader header;
	//���b�V��
	std::vector<ModelMesh>meshes;
	//�X�P���g���i�{�[���\���j
	Skeleton skelton;

	Model(const std::string& Dir, const std::string& FileName) :header(Dir, FileName) {}

	void MeshSmoothing()
	{
		for (auto& m : meshes)
		{
			m.Smoothing();
		}
	}
};

