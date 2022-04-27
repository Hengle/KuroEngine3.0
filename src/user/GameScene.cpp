#include "GameScene.h"
#include"KuroEngine.h"

static const Vec2<float>INIT_VEL = { 10,10 };
static const float RADIUS = 32.0f;

GameScene::GameScene()
{

}

void GameScene::OnInitialize()
{
	pos = WinApp::Instance()->GetExpandWinCenter();

	if (!accelMove)
	{
		vel = INIT_VEL;
	}
	else
	{
		vel = { 0,0 };
		accel = { 0,0 };
	}
}

#include"KuroMath.h"
void GameScene::OnUpdate()
{
	bool up = UsersInput::Instance()->KeyInput(DIK_W);
	bool down = UsersInput::Instance()->KeyInput(DIK_S);
	bool left = UsersInput::Instance()->KeyInput(DIK_A);
	bool right = UsersInput::Instance()->KeyInput(DIK_D);
	float skewRate = cos(Angle::ConvertToRadian(45));

	//等速移動
	if (!accelMove)
	{
		if (left)
		{
			pos.x -= vel.x * ((up || down) ? skewRate : 1.0f);
		}
		if (right)
		{
			pos.x += vel.x * ((up || down) ? skewRate : 1.0f);
		}
		if (up)
		{
			pos.y -= vel.y * ((left || right) ? skewRate : 1.0f);
		}
		if (down)
		{
			pos.y += vel.y * ((left || right) ? skewRate : 1.0f);
		}
	}
	//加速で移動
	else
	{
		static const float ACCEL = 0.2f;
		if (left)
		{
			accel.x -= ACCEL * ((up || down) ? skewRate : 1.0f);
		}
		if (right)
		{
			accel.x += ACCEL * ((up || down) ? skewRate : 1.0f);
		}


		//重力が有効
		if (addGravity)
		{
			static const float GRAVITY = 0.1f;
			accel.y += GRAVITY;

			//ジャンプ
			static const float JUMP_POWER = -3.0f;
			if (UsersInput::Instance()->KeyOnTrigger(DIK_SPACE))
			{
				accel.y = JUMP_POWER;
			}
		}
		//WSキー有効（トップビュー移動）
		else
		{
			if (up)
			{
				accel.y -= ACCEL * ((left || right) ? skewRate : 1.0f);
			}
			if (down)
			{
				accel.y += ACCEL * ((left || right) ? skewRate : 1.0f);
			}
		}

		//速度加算
		vel += accel;
		pos += vel;

		//速度の減衰
		vel = KuroMath::Lerp(vel, { 0,0 }, 0.3f);
	}

	//画面外に出ないよう制限
	const auto winSize = WinApp::Instance()->GetExpandWinSize();
	static const auto REFLECT_RATE = -0.8f;
	if (winSize.x < pos.x + RADIUS)
	{
		pos.x = winSize.x - RADIUS;
		accel.x = 0.0f;
		vel.x *= REFLECT_RATE;
	}
	if (pos.x - RADIUS < 0.0f)
	{
		pos.x = RADIUS;
		accel.x = 0.0f;
		vel.x *= REFLECT_RATE;
	}
	if (winSize.y < pos.y + RADIUS)
	{
		pos.y = winSize.y - RADIUS;
		accel.y = 0.0f;
		vel.y *= REFLECT_RATE;
	}
	if (pos.y - RADIUS < 0.0f)
	{
		pos.y = RADIUS;
		accel.y = 0.0f;
		vel.y *= REFLECT_RATE;
	}
}

#include"DrawFunc2D.h"
void GameScene::OnDraw()
{
	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() });
	DrawFunc2D::DrawCircle2D(pos, RADIUS, Color(), true);
}

void GameScene::OnImguiDebug()
{
	ImGui::Begin("Parameters");

	if (ImGui::Checkbox("AccelMove", &accelMove))
	{
		if (!accelMove)vel = INIT_VEL;
		else
		{
			vel = { 0,0 };
			accel = { 0,0 };
		}
	}

	ImGui::Indent(32.0f);

	if (ImGui::Checkbox("AddGravoty", &addGravity))
	{
		accel = { 0,0 };
		vel = { 0,0 };
		if (!accelMove)accelMove = true;
	}

	ImGui::Separator();

	if (addGravity)
	{
		ImGui::Text("Move - AD");
		ImGui::Text("Space - Jump");
	}
	else
	{
		ImGui::Text("Move - WASD");
	}

	ImGui::End();
}

void GameScene::OnFinalize()
{
}
