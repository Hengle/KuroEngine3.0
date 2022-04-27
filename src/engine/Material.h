#pragma once
#include"Vec.h"
#include<string>
#include<memory>

class ConstantBuffer;
class TextureBuffer;
static const enum MATERIAL_TEX_TYPE { COLOR_TEX, EMISSIVE_TEX, NORMAL_TEX, MATERIAL_TEX_TYPE_NUM };

struct Material
{
private:
	bool invalid = true;
public:
	std::string name = "DefaultMaterial";
	struct ConstData
	{
		//Lambert
		Vec3<float> ambient = { 1, 1, 1 };	//環境光(全方向から当たる光)
		float ambientFactor = 1.0f;			//強度(weight)
		Vec3<float> diffuse = { 0, 0, 0 };	//拡散反射光(モデルが本来持っている色味)
		float diffuseFactor = 1.0f;
		Vec3<float> emissive = { 0,0,0 };		//放射光(モデル自身が放つ光、暗い環境で光る)
		float emissiveFactor = 1.0f;
		float transparent = 0.0f;				//透過度

		//Phong
		Vec3<float> specular = { 0,0,0 };	//スペキュラ(モデルの艶)
		float specularFactor = 1.0f;
		float shininess = 0.0f;					//光沢
		float reflection = 0.0f;					//反射(値が大きいと鏡のようになる)
	}constData;
	std::shared_ptr<ConstantBuffer>buff;

	std::shared_ptr<TextureBuffer> texBuff[MATERIAL_TEX_TYPE_NUM];

	Material();
	void CreateBuff();
};