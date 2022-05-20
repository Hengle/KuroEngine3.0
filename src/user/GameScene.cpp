#include "GameScene.h"
#include"KuroEngine.h"
#include"Importer.h"
#include"DrawFunc3D.h"
#include"Camera.h"
#include"Model.h"
#include"Object.h"

GameScene::GameScene()
{
	skyDome = Importer::Instance()->LoadModel("resource/user/", "skydome.glb");
	floor = Importer::Instance()->LoadModel("resource/user/", "floor.glb");
	testModel = std::make_shared<ModelObject>("resource/user/", "player.glb");
	testModel->transform.SetPos({ 0,1.5f,0 });

	//dirLig.SetDir(Vec3<Angle>(50, -30, 0));
	ligMgr.RegisterDirLight(&dirLig);
	ligMgr.RegisterPointLight(&ptLig);
	ligMgr.RegisterHemiSphereLight(&hemiLig);

	//trans.SetRotate(Vec3<Angle>(-90, 0, 0));
	//trans.SetScale(0.3f);
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

	//ポイントライト位置
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

	//ライトのON/OFF
	if (UsersInput::Instance()->KeyOnTrigger(DIK_1))
	{
		dirLig.SetActive();
	}
	if (UsersInput::Instance()->KeyOnTrigger(DIK_2))
	{
		ptLig.SetActive();
	}
	if (UsersInput::Instance()->KeyOnTrigger(DIK_3))
	{
		hemiLig.SetActive();
	}

	debugCam.Move();
}


void GameScene::OnDraw()
{
	static std::shared_ptr<DepthStencil>dsv = D3D12App::Instance()->GenerateDepthStencil(
		D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize());

	dsv->Clear(D3D12App::Instance()->GetCmdList());

	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() }, dsv);

	DrawFunc3D::DrawNonShadingModel(skyDome, trans, debugCam);
	DrawFunc3D::DrawADSShadingModel(ligMgr, floor, trans, debugCam);
	DrawFunc3D::DrawADSShadingModel(ligMgr, testModel, debugCam);
}

void GameScene::OnImguiDebug()
{
}

void GameScene::OnFinalize()
{
}
