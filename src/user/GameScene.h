#pragma once
#include"KuroEngine.h"
#include"DebugCamera.h"
#include"LightManager.h"
#include"Transform.h"
#include<array>

class Model;

class GameScene : public BaseScene
{
	const enum MODEL
	{
		MONKEY_GLB,
		DRAGON_FBX,
		PLAYER_GLB,
		METAL_BALL_FBX,
		WOOD_CUBE_FBX,
		MODEL_NUM,
		DEFAULT = WOOD_CUBE_FBX
	};
	MODEL nowModel = DEFAULT;
	std::array<std::shared_ptr<Model>, MODEL_NUM>models;

	DebugCamera debugCam;
	LightManager ligMgr;
	Light::Direction dirLig;
	Light::Point ptLig;
	Transform trans;
public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};