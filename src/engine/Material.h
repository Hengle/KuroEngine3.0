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
		Vec3<float> ambient = { 1, 1, 1 };	//����(�S�������瓖�����)
		float ambientFactor = 1.0f;			//���x(weight)
		Vec3<float> diffuse = { 0, 0, 0 };	//�g�U���ˌ�(���f�����{�������Ă���F��)
		float diffuseFactor = 1.0f;
		Vec3<float> emissive = { 0,0,0 };		//���ˌ�(���f�����g�������A�Â����Ō���)
		float emissiveFactor = 1.0f;
		float transparent = 0.0f;				//���ߓx

		//Phong
		Vec3<float> specular = { 0,0,0 };	//�X�y�L����(���f���̉�)
		float specularFactor = 1.0f;
		float shininess = 0.0f;					//����
		float reflection = 0.0f;					//����(�l���傫���Ƌ��̂悤�ɂȂ�)
	}constData;
	std::shared_ptr<ConstantBuffer>buff;

	std::shared_ptr<TextureBuffer> texBuff[MATERIAL_TEX_TYPE_NUM];

	Material();
	void CreateBuff();
};