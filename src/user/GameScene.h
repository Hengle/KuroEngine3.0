#pragma once
#include"KuroEngine.h"
#include"DebugCamera.h"
#include"LightManager.h"
class GameScene : public BaseScene
{
	DebugCamera debugCam;
	LightManager ligMgr;
	Light::Direction dirLig;
	Light::Point ptLig;
public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};