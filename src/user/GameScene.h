#pragma once
#include"KuroEngine.h"
#include"LightManager.h"

#include<memory>

class GameScene : public BaseScene
{
	Vec2<float>pos;
	Vec2<float>vel;

	bool accelMove = false;
	Vec2<float>accel;

	bool addGravity = false;

public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};