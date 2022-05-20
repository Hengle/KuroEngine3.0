#pragma once
#include"KuroEngine.h"
#include"DebugCamera.h"
#include"LightManager.h"
#include"Transform.h"
#include<array>

class Model;
class ModelObject;
class GaussianBlur;

class GameScene : public BaseScene
{
	std::shared_ptr<Model>skyDome;
	std::shared_ptr<Model>floor;
	std::shared_ptr<ModelObject>testModel;

	DebugCamera debugCam;
	LightManager ligMgr;
	Light::Direction dirLig;
	Light::Point ptLig;
	Light::HemiSphere hemiLig;
	Transform trans;

	std::shared_ptr<RenderTarget>shadowMap;
	std::shared_ptr<DepthStencil>shadowMapDepth;
	Camera lightCamera;
	std::shared_ptr<GaussianBlur> gaussianBlur;
public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};