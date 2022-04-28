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
		static std::vector<InputLayoutParam>GetInputLayout()
		{
			static std::vector<InputLayoutParam>INPUT_LAYOUT =
			{
				InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT),
				InputLayoutParam("NORMAL",DXGI_FORMAT_R32G32B32_FLOAT),
				InputLayoutParam("TEXCOORD",DXGI_FORMAT_R32G32_FLOAT),
				InputLayoutParam("BONE_NO",DXGI_FORMAT_R16G16B16A16_SINT),
				InputLayoutParam("WEIGHT",DXGI_FORMAT_R32G32B32A32_FLOAT),
			};
			return INPUT_LAYOUT;
		}

		Vec3<float>pos;
		Vec3<float>normal;
		Vec2<float>uv;
		Vec4<signed short>boneIdx = { -1,-1,-1,-1 };
		Vec4<float>boneWeight = { 0,0,0,0 };
	};

	std::shared_ptr<Mesh<Vertex_Model>>mesh;
	std::shared_ptr<Material> material;

	void Smoothing();
};

