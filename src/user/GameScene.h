#pragma once
#include"KuroEngine.h"
#include"LightManager.h"
#include<memory>

class GameScene : public BaseScene
{
	struct Ball
	{
		const Vec2<float>initPos;
		Vec2<float>pos;
		Vec2<float>vel;
		const float mass;
		const float radius;
		bool hold;
		Ball(const Vec2<float>& Pos, const float& Mass,const float& Radius) :initPos(Pos), mass(Mass),radius(Radius) { Init(); }
		void Init()
		{
			pos = initPos;
			vel = { 0,0 };
			hold = false;
		}
	};
	static const int BALL_NUM = 3;
	std::array<std::shared_ptr<Ball>, BALL_NUM>balls;

	std::shared_ptr<Ball>CheckHitMousePoint();
	void CheckHitBallUpdate(std::weak_ptr<Ball>CheckBall);
	std::weak_ptr<Ball>holdBall;

public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};