#include "Material.h"
#include"D3D12App.h"

Material::Material()
{
	static std::shared_ptr<TextureBuffer> DEFAULT[MATERIAL_TEX_TYPE_NUM] =
	{
		D3D12App::Instance()->GenerateTextureBuffer(Color(1.0f, 1.0f, 1.0f, 1.0f)),
		D3D12App::Instance()->GenerateTextureBuffer(Color(0.0f, 0.0f, 0.0f, 1.0f)),
		D3D12App::Instance()->GenerateTextureBuffer(Color(0.0f, 0.0f, -1.0f, 1.0f)),
	};
	for (int i = 0; i < MATERIAL_TEX_TYPE_NUM; ++i)
	{
		texBuff[i] = DEFAULT[i];
	}
}

void Material::CreateBuff()
{
	//Šù‚É¶¬Ï‚Ý
	if (!invalid)return;
	buff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(ConstData), 1, &constData, name.c_str());
	
	invalid = false;
}

void Material::Mapping()
{
	buff->Mapping(&constData);
}
