#include "GameScene.h"
#include"KuroEngine.h"
#include<algorithm>

std::shared_ptr<GameScene::Ball> GameScene::CheckHitMousePoint()
{
	const auto mousePos = UsersInput::Instance()->GetMousePos();

	for (auto& b : balls)
	{
		float dist = mousePos.Distance(b->pos);
		if (dist < b->radius)
		{
			b->hold = true;
			b->vel = { 0,0 };
			return b;
		}
	}
	return std::shared_ptr<Ball>();
}

void GameScene::CheckHitBallUpdate(std::weak_ptr<Ball> CheckBall)
{
	auto checkBall = CheckBall.lock();
	if (checkBall->vel.IsZero())return;

	std::shared_ptr<Ball>hit;
	float shortest = 10000000.0f;

	for (auto& b : balls)
	{
		//自分自身ならスルー
		if (b == checkBall)continue;

		const auto ps = checkBall->pos;
		const auto pe = checkBall->pos + checkBall->vel;

		const auto pc = b->pos;

		const auto startToSphere = pc - ps;
		const auto startToEnd = pe - ps;
		const auto nearLength = startToEnd.GetNormal().Dot(startToSphere);
		const auto nearLengthRate = nearLength / startToEnd.Length();
		const auto nearPt = ps + startToEnd * std::clamp(nearLengthRate, 0.0f, 1.0f);

		const auto endToSphere = pc - pe;
		const auto nearToSphere = pc - nearPt;

		float dist = 0.0f;
		if (nearLengthRate < 0.0f)
		{
			dist = startToSphere.Length();
		}
		else if (1.0f < nearLengthRate)
		{
			dist = endToSphere.Length();
		}
		else
		{
			dist = nearToSphere.Length();
		}

		if (dist - checkBall->radius - b->radius <= 0 && dist < shortest)
		{
			shortest = dist;
			hit = b;
		}
	}

	//ボール同士の反発係数
	static const float REBOUND_E = 0.2f;

	if (hit)
	{
		const auto m1 = checkBall->mass;
		const auto v1 = checkBall->vel;
		const auto m2 = hit->mass;
		const auto v2 = hit->vel;
		const auto e = REBOUND_E;

		//衝突後の相手の速度
		const auto v2_ = (v1 * (1 + e) * m1 + v2 * m2 + v2 * m1 * e) / (m2 - e);
		const auto v1_ = -(v1 - v2) * e - v2_;

		//押し戻し
		//checkBall->pos = -v1.GetNormal() * (checkBall->radius + hit->radius + 1) + hit->pos;
		checkBall->vel = v1_;
		hit->vel = v2_;
	}
	else
	{
		checkBall->pos += checkBall->vel;
		//空気抵抗でだんだん原則
		checkBall->vel *= 0.98f;
	}
}

GameScene::GameScene()
{
	const float massA = 4.0f;
	const float radiusA = 48.0f;
	const float massB = 3.0f;
	const float radiusB = 32.0f;

	const float offsetX = 200.0f;
	const auto winCenter = WinApp::Instance()->GetExpandWinCenter();


	balls[0] = std::make_shared<Ball>(winCenter + Vec2<float>(0.0f, 0.0f), massA, radiusA);
	balls[1] = std::make_shared<Ball>(winCenter + Vec2<float>(-offsetX, 0.0f), massB, radiusB);
	balls[2] = std::make_shared<Ball>(winCenter + Vec2<float>(offsetX, 0.0f), massB, radiusB);
}

void GameScene::OnInitialize()
{
	for (auto& b : balls)
	{
		b->Init();
	}
}

#include"KuroMath.h"
void GameScene::OnUpdate()
{
	static const float REBOUND_E = 0.8f;	//壁との反発係数
	static const float VEL_BASE_DIST = 10.0f;	//速さの基準となる距離

	const auto mousePos = UsersInput::Instance()->GetMousePos();
	const auto winSize = WinApp::Instance()->GetExpandWinSize();

	for (auto& b : balls)
	{
		if (!b->hold)
		{
			CheckHitBallUpdate(b);
		}

		//左画面端
		if (b->pos.x - b->radius < 0.0f)
		{
			b->pos.x = b->radius;
			b->vel.x *= -REBOUND_E;
		}
		//右画面端
		else if (winSize.x < b->pos.x + b->radius)
		{
			b->pos.x = winSize.x - b->radius;
			b->vel.x *= -REBOUND_E;
		}
		//上画面端
		if (b->pos.y - b->radius < 0.0f)
		{
			b->pos.y = b->radius;
			b->vel.y *= -REBOUND_E;
		}
		//下画面端
		else if (winSize.y < b->pos.y + b->radius)
		{
			b->pos.y = winSize.y - b->radius;
			b->vel.y *= -REBOUND_E;
		}
	}

	if (UsersInput::Instance()->MouseOnTrigger(MOUSE_BUTTON::LEFT))
	{
		holdBall = CheckHitMousePoint();
	}

	if (auto b = holdBall.lock())
	{
		if (UsersInput::Instance()->MouseOffTrigger(MOUSE_BUTTON::LEFT))
		{
			b->vel = -(mousePos - b->pos).GetNormal() * mousePos.Distance(b->pos) / VEL_BASE_DIST;

			b->hold = false;
			holdBall.reset();
		}

		//最初に計算
		CheckHitBallUpdate(b);
	}

	if (UsersInput::Instance()->KeyOnTrigger(DIK_I))
	{
		this->Initialize();
	}



	
}

#include"DrawFunc2D.h"
void GameScene::OnDraw()
{
	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() });

	for (auto& b : balls)
	{
		DrawFunc2D::DrawCircle2D(b->pos, b->radius, b->hold ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color(), true);
	}

	if (auto b = holdBall.lock())
	{
		DrawFunc2D::DrawLine2D(b->pos, UsersInput::Instance()->GetMousePos(), Color(1.0f, 0.0f, 0.0f, 1.0f));
	}
}

void GameScene::OnImguiDebug()
{
}

void GameScene::OnFinalize()
{
}
