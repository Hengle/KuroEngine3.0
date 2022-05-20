#pragma once
#include"KuroEngine.h"
#include"DebugCamera.h"
#include"LightManager.h"
#include"Transform.h"
#include<array>

class Model;
class ModelObject;

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
public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};