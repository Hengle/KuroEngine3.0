#pragma once
#include"KuroEngine.h"
#include"DebugCamera.h"
#include"LightManager.h"
#include"Transform.h"

class Model;

class GameScene : public BaseScene
{
	DebugCamera debugCam;
	LightManager ligMgr;
	Light::Direction dirLig;
	Light::Point ptLig;
	Transform trans;

	std::shared_ptr<Model>playerModel;
	std::shared_ptr<Model>drawTest;
public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};