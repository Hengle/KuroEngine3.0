#pragma once
#include"Vec.h"
#include<memory>
#include"Mesh.h"
#include"Material.h"

class ModelMesh
{
public:
	//モデル専用頂点クラス
	class Vertex_Model
	{
	public:
		Vec3<float>pos;
		Vec3<float>normal;
		Vec2<float>uv;
		Vec4<signed short>boneIdx = { -1,-1,-1,-1 };
		Vec4<float>boneWeight = { 0,0,0,0 };
	};

	std::shared_ptr<Mesh<Vertex_Model>>mesh;
	std::shared_ptr<Material> material;
};

