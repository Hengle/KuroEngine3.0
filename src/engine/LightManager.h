#pragma once
#include<memory>
#include<vector>
#include"Light.h"
#include<wrl.h>
#include<d3d12.h>

class ConstantBuffer;
class StructuredBuffer;
class DescriptorData;

class LightManager
{
private:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static const int MAX_LIG_NUM[Light::TYPE_NUM];
private:
	std::vector<Light::Direction*>dirLights;
	std::vector<Light::Point*>ptLights;
	std::vector<Light::Spot*>spotLights;
	std::vector<Light::HemiSphere*>hemiSphereLights;

	//アクティブ中のライトの数を送るための定数バッファデータ
	struct LightNum
	{
		int dirLigNum;
		int ptLigNum;
		int spotLigNum;
		int hemiSphereNum;
	};
	std::shared_ptr<ConstantBuffer>ligNumConstBuff;

	//ライト情報用構造化バッファ
	std::shared_ptr<StructuredBuffer>ligStructuredBuff[Light::TYPE_NUM];

	//アクティブ中のライトの数をマッピング
	void MappingLigNum();
	// (引数で指定したタイプの) ライト情報をマッピング、Dirtyフラグを確認するか
	void MappingLigInfo(const Light::TYPE& Type, const bool& CheckDirty = true);

public:
	LightManager();

	void RegisterDirLight(Light::Direction* DirLight);
	void RegisterPointLight(Light::Point* PtLight);
	void RegisterSpotLight(Light::Spot* SpotLight);
	void RegisterHemiSphereLight(Light::HemiSphere* HemiSphereLight);

	std::shared_ptr<DescriptorData>GetLigNumInfo();
	std::shared_ptr<DescriptorData>GetLigInfo(const Light::TYPE& Type);
};