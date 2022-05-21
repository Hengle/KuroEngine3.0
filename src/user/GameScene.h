#pragma once
#include"KuroEngine.h"
#include"DebugCamera.h"
#include"LightManager.h"
#include"Transform.h"
#include<array>
#include"ShadowMapDevice.h"

class Model;
class ModelObject;
class GaussianBlur;

class GameScene : public BaseScene
{
	std::shared_ptr<Model>skyDome;
	std::shared_ptr<ModelObject>floor;
	std::shared_ptr<ModelObject>testModel;

	DebugCamera debugCam;
	LightManager ligMgr;
	Light::Direction dirLig;
	Light::Point ptLig;
	Light::HemiSphere hemiLig;
	Transform trans;
	ShadowMapDevice shadowMapDevice;
public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};