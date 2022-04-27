#include "GameScene.h"
#include"KuroEngine.h"

static const Vec2<float>INIT_VEL = { 10,10 };
static const float RADIUS = 32.0f;

void GameScene::EmitParticles(const int& Num, const Vec2<float>& EmitPos, const Vec2<float>& EmitVec, const Angle& RandAngle)
{
	int num = 0;
	for (auto& p : particles)
	{
		if (p.life)continue;
		float rotateAngle = KuroFunc::GetRand(-RandAngle / 2.0f, RandAngle / 2.0f);
		p.Emit(EmitPos, KuroMath::RotateVec2(EmitVec, rotateAngle));
		num++;
		if (Num <= num)break;
	}
}

GameScene::GameScene()
{

}

void GameScene::OnInitialize()
{
	pos = WinApp::Instance()->GetExpandWinCenter();

	vel = { 0,0 };
	accel = { 0,0 };

	for (auto& p : particles)
	{
		p.Init();
	}
}

#include"KuroMath.h"
void GameScene::OnUpdate()
{
	static const float GRAVITY = 0.1f;
	static const float MAX_ACCEL_Y = 5.0f;
	static const float AIR_FRICTION = 0.99f;

	//パーティクル更新
	for (auto& p : particles)
	{
		if (!p.life)continue;

		//空気抵抗X
		p.accel.x *= AIR_FRICTION * 0.9f;

		//重力
		p.accel.y += GRAVITY;

		//最大落下加速度
		if (MAX_ACCEL_Y < p.accel.y)p.accel.y = MAX_ACCEL_Y;

		p.vel += p.accel;
		//速度の減衰
		p.vel = KuroMath::Lerp(p.vel, { 0,0 }, 0.3f);

		p.pos += p.vel;

		p.life--;
	}

	//プレイヤー入力と更新
	bool up = UsersInput::Instance()->KeyInput(DIK_W);
	bool down = UsersInput::Instance()->KeyInput(DIK_S);
	bool left = UsersInput::Instance()->KeyInput(DIK_A);
	bool right = UsersInput::Instance()->KeyInput(DIK_D);
	float skewRate = cos(Angle::ConvertToRadian(45));

	static const float ACCEL = 0.5f;
	//地面との摩擦
	const float groundFrictionRate = onGround ? 1.0f : 0.08f;
	if (left)
	{
		accel.x -= ACCEL * groundFrictionRate * ((up || down) ? skewRate : 1.0f);

		if (onGround)
		{
			EmitParticles(1, { pos.x,pos.y + RADIUS }, Vec2<float>(1.0f, -1.0f), Angle(45));
		}
	}
	if (right)
	{
		accel.x += ACCEL * groundFrictionRate * ((up || down) ? skewRate : 1.0f);

		if (onGround)
		{
			EmitParticles(1, { pos.x,pos.y + RADIUS }, Vec2<float>(-1.0f, -1.0f), Angle(45));
		}
	}

	//重力
	accel.y += GRAVITY;

	//最大重力加速度制限（空気抵抗Y）
	if (MAX_ACCEL_Y < accel.y)
	{
		accel.y = MAX_ACCEL_Y;
	}

	//ジャンプ（空中ジャンプ可）
	static const float JUMP_POWER = -2.5f;
	if (UsersInput::Instance()->KeyOnTrigger(DIK_SPACE))
	{
		accel.y = JUMP_POWER;
	}

	//速度加算
	vel += accel;
	pos += vel;

	//速度の減衰
	vel = KuroMath::Lerp(vel, { 0,0 }, 0.3f);

	//画面外に出ないよう制限
	const auto winSize = WinApp::Instance()->GetExpandWinSize();
	static const auto REFLECT_RATE = -0.8f;

	//壁との摩擦
	static const float WALL_FRICTION_RATE = 0.8f;
	bool wallHit = false;

	//右
	if (winSize.x < pos.x + RADIUS)
	{
		pos.x = winSize.x - RADIUS;
		accel.x = 0.0f;
		accel.y *= WALL_FRICTION_RATE;	//摩擦
		vel.x *= REFLECT_RATE;
		wallHit = true;
		EmitParticles(1, { pos.x + RADIUS,pos.y }, Vec2<float>(-1.0f, -1.0f), Angle(45));
	}
	//左
	if (pos.x - RADIUS < 0.0f)
	{
		pos.x = RADIUS;
		accel.x = 0.0f;
		accel.y *= WALL_FRICTION_RATE;	//摩擦
		vel.x *= REFLECT_RATE;
		wallHit = true;
		EmitParticles(1, { pos.x - RADIUS,pos.y }, Vec2<float>(1.0f, -1.0f), Angle(45));
	}
	//下
	if (winSize.y < pos.y + RADIUS)
	{
		pos.y = winSize.y - RADIUS;
		accel.y = 0.0f;
		accel.x *= WALL_FRICTION_RATE;	//摩擦
		vel.y *= REFLECT_RATE;
		if (!onGround)
		{
			EmitParticles(1, { pos.x,pos.y + RADIUS }, Vec2<float>(0.0f, -1.0f), Angle(45));
		}
		onGround = true;
		wallHit = true;
	}
	else onGround = false;
	//上
	if (pos.y - RADIUS < 0.0f)
	{
		pos.y = RADIUS;
		accel.y = 0.0f;
		accel.x *= WALL_FRICTION_RATE;	//摩擦
		vel.y *= REFLECT_RATE;
		wallHit = true;
		EmitParticles(1, { pos.x,pos.y - RADIUS }, Vec2<float>(0.0f, 1.0f), Angle(45));
	}

	//空気抵抗X
	if (!wallHit)
	{
		accel.x *= AIR_FRICTION;
	}
}

#include"DrawFunc2D.h"
void GameScene::OnDraw()
{
	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() });

	for (auto& p : particles)
	{
		if (!p.life)continue;

		static const float MAX_SIZE = 10.0f;
		const float lifeRate = (p.lifeSpan - p.life) / (float)p.lifeSpan;
		const float size = KuroMath::Ease(In, Circ, lifeRate, MAX_SIZE, 0.0f);
		const Vec2<float>leftUpPos = p.pos - Vec2<float>(size, size);
		const Vec2<float>rightBottomPos = p.pos + Vec2<float>(size, size);
		DrawFunc2D::DrawBox2D(leftUpPos, rightBottomPos, Color(1.0f, 1.0f, 1.0f, 1.0f - lifeRate), true, AlphaBlendMode_Trans);
	}

	DrawFunc2D::DrawCircle2D(pos, RADIUS, Color(), true);
}

void GameScene::OnImguiDebug()
{
	ImGui::Begin("Parameters");

	ImGui::Text("Move - AD");
	ImGui::Text("Space - Jump");

	ImGui::End();
}

void GameScene::OnFinalize()
{
}
