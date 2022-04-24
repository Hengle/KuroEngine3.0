#include "GameScene.h"
#include"KuroEngine.h"
#include"Importer.h"

GameScene::GameScene()
{
}

void GameScene::OnInitialize()
{
}

void GameScene::OnUpdate()
{
}

#include"DrawFunc3D.h"
#include"Camera.h"
void GameScene::OnDraw()
{
	static auto testModel = Importer::Instance()->LoadFBXModel("resource/user/player/", "player.fbx");
	static Camera cam("test");
	static Transform trans;
	static std::shared_ptr<DepthStencil>dsv = D3D12App::Instance()->GenerateDepthStencil(
		D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize());

	dsv->Clear(D3D12App::Instance()->GetCmdList());

	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() }, dsv);
	DrawFunc3D::DrawNonShadingModel(testModel, trans, cam);

}

void GameScene::OnImguiDebug()
{
}

void GameScene::OnFinalize()
{
}
