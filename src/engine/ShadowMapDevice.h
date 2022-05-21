#pragma once
#include"Camera.h"
#include<memory>
#include<vector>
#include"D3D12Data.h"
class GaussianBlur;
class ModelObject;
class ShadowMapDevice
{
	std::shared_ptr<RenderTarget>shadowMap;
	std::shared_ptr<DepthStencil>shadowMapDepth;
	Camera lightCamera;
	std::shared_ptr<GaussianBlur> gaussianBlur;

public:
	ShadowMapDevice();
	void DrawShadowMap(const std::vector<std::weak_ptr<ModelObject>>& Models);
	void DrawShadowReceiver(const std::vector<std::weak_ptr<ModelObject>>& Models, Camera& GameCamera, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
};

