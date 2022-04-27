#include "GameScene.h"
#include"KuroEngine.h"
#include"Importer.h"

GameScene::GameScene()
{
	ligMgr.RegisterDirLight(&dirLig);
	ligMgr.RegisterPointLight(&ptLig);
}

void GameScene::OnInitialize()
{
	debugCam.Init({ 0,1,-3 }, { 0,1,0 });
}

void GameScene::OnUpdate()
{
	if (UsersInput::Instance()->KeyOnTrigger(DIK_I))
	{
		debugCam.Init({ 0,1,-3 }, { 0,1,0 });
	}

	static const float UINT = 0.1f;
	auto ptLigPos = ptLig.GetPos();
	if (UsersInput::Instance()->KeyInput(DIK_W))
	{
		ptLigPos.y += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_S))
	{
		ptLigPos.y -= UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_D))
	{
		ptLigPos.x += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_A))
	{
		ptLigPos.x -= UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_E))
	{
		ptLigPos.z += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_Q))
	{
		ptLigPos.z -= UINT;
	}
	ptLig.SetPos(ptLigPos);

	debugCam.Move();
}

#include"DrawFunc3D.h"
#include"Camera.h"
void GameScene::OnDraw()
{
	static auto testModel = Importer::Instance()->LoadFBXModel("resource/user/player/", "player.fbx");
	static auto testglTF0 = Importer::Instance()->LoadGLTFModel("resource/user/gltf/", "player.glb");
	//static auto testglTF1 = Importer::Instance()->LoadGLTFModel("resource/user/gltf/", "player.gltf");
	//static auto testglTF2 = Importer::Instance()->LoadGLTFModel("resource/user/gltf/", "player_anim_test.gltf");

	static Transform trans;
	static std::shared_ptr<DepthStencil>dsv = D3D12App::Instance()->GenerateDepthStencil(
		D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize());

	dsv->Clear(D3D12App::Instance()->GetCmdList());

	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() }, dsv);

	static bool DRAW_FLG = false;
	if (DRAW_FLG)
	{
		DrawFunc3D::DrawShadingModel(ligMgr, testglTF0, trans, debugCam);
	}
	else
	{
		DrawFunc3D::DrawShadingModel(ligMgr, testModel, trans, debugCam);
	}

	if (UsersInput::Instance()->KeyOnTrigger(DIK_R))DRAW_FLG = !DRAW_FLG;


	//DrawFunc3D::DrawNonShadingModel(testModel, trans, debugCam);
	//DrawFunc3D::DrawNonShadingModel(testglTF0, trans, debugCam);
	DrawFunc3D::DrawLine(debugCam, { 0,0,0 }, ptLig.GetPos(), Color(1.0f, 0.0f, 0.0f, 1.0f), 0.3f);
}

void GameScene::OnImguiDebug()
{
}

void GameScene::OnFinalize()
{
}
