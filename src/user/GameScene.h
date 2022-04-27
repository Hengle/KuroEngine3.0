#pragma once
#include"KuroEngine.h"
#include"LightManager.h"
#include<memory>
#include<array>

class GameScene : public BaseScene
{
	struct Particle
	{
		Vec2<float>pos;
		Vec2<float>vel;
		Vec2<float>accel;
		int life;
		int lifeSpan;
		void Init() { life = 0; }
		void Emit(const Vec2<float>& EmitPos, const Vec2<float>& EmitVec)
		{
			pos = EmitPos;
			vel = { 0,0 };
			static const float POWER_MAX = 3.0f;
			static const float POWER_MIN = 1.0f;
			accel = EmitVec * KuroFunc::GetRand(POWER_MIN, POWER_MAX);
			static const int LIFE_MAX = 30;
			static const int LIFE_MIN = 8;
			lifeSpan = KuroFunc::GetRand(LIFE_MIN, LIFE_MAX);
			life = lifeSpan;
		}
	};
	static const int PARTICLE_MAX = 100;
	std::array<Particle, PARTICLE_MAX>particles;

	Vec2<float>pos;
	Vec2<float>vel;

	Vec2<float>accel;

	bool onGround = false;

	void EmitParticles(const int& Num, const Vec2<float>& EmitPos, const Vec2<float>& EmitVec, const Angle& RandAngle);

public:
	GameScene();
	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
};