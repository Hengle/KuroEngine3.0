#include "ModelMesh.h"

void ModelMesh::Smoothing()
{
	mesh->vertBuff.reset();

	std::vector<std::vector<unsigned int>>sameVertexIndices;
	std::vector<char>flg;
	flg.resize(mesh->vertices.size());
	std::fill(flg.begin(), flg.end(), 0);
	for (int i = 0; i < mesh->vertices.size(); ++i)
	{
		if (flg[i] != 0)continue;
		flg[i] = 1;
		sameVertexIndices.emplace_back();
		sameVertexIndices.back().emplace_back(i);
		for (int j = i + 1; j < mesh->vertices.size(); ++j)
		{
			if (mesh->vertices[i].pos == mesh->vertices[j].pos)
			{
				sameVertexIndices.back().emplace_back(j);
				flg[j] = 1;
			}
		}
	}
	for (auto sameVertex : sameVertexIndices)
	{
		Vec3<float>norm(0, 0, 0);
		for (auto vIdx : sameVertex)
		{
			norm += mesh->vertices[vIdx].normal;
		}
		norm /= sameVertex.size();
		for (auto vIdx : sameVertex)
		{
			mesh->vertices[vIdx].normal = norm;
		}
	}

	mesh->CreateBuff();
}
