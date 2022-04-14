#include "GameScene.h"
#include"KuroEngine.h"

GameScene::GameScene()
{

}

void GameScene::OnInitialize()
{
}

void GameScene::OnUpdate()
{
}

#include"DrawFunc.h"
void GameScene::OnDraw()
{
	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() });
}

void GameScene::OnImguiDebug()
{
}

void GameScene::OnFinalize()
{
}
