#include "GameScene.h"
#include"KuroEngine.h"
#include"Importer.h"
#include"DrawFunc3D.h"
#include"DrawFunc2D.h"
#include"Camera.h"
#include"Model.h"
#include"Object.h"

GameScene::GameScene() :lightCamera("LightCamera")
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

	//シャドウマップ関連
	shadowMap = D3D12App::Instance()->GenerateRenderTarget(DXGI_FORMAT_R32_FLOAT, Color(), { 1024,1024 }, L"ShadowMap");
	shadowMapDepth = D3D12App::Instance()->GenerateDepthStencil({ 1024,1024 });
	lightCamera.SetPos({ 0, 10, 0 });
	lightCamera.SetTarget({ 0,0,0 });
	lightCamera.SetUp({ 1,0,0 });
	lightCamera.SetAngleOfView(Angle(60));
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
	if (UsersInput::Instance()->KeyInput(DIK_RIGHT))
	{
		ptLigPos.x += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_LEFT))
	{
		ptLigPos.x -= UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_UP))
	{
		ptLigPos.z += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_DOWN))
	{
		ptLigPos.z -= UINT;
	}
	ptLig.SetPos(ptLigPos);

	//テストモデルの位置
	auto modelPos = testModel->transform.GetPos();
	if (UsersInput::Instance()->KeyInput(DIK_E))
	{
		modelPos.y += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_Q))
	{
		modelPos.y -= UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_D))
	{
		modelPos.x += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_A))
	{
		modelPos.x -= UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_W))
	{
		modelPos.z += UINT;
	}
	if (UsersInput::Instance()->KeyInput(DIK_S))
	{
		modelPos.z -= UINT;
	}
	testModel->transform.SetPos(modelPos);

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
	shadowMap->Clear(D3D12App::Instance()->GetCmdList());
	shadowMapDepth->Clear(D3D12App::Instance()->GetCmdList());

	//シャドウマップ書き込み
	KuroEngine::Instance().Graphics().SetRenderTargets({ shadowMap }, shadowMapDepth);
	DrawFunc3D::DrawShadowMapModel(testModel->model, testModel->transform, lightCamera);

	//標準描画
	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() }, dsv);
	DrawFunc3D::DrawNonShadingModel(skyDome, trans, debugCam);
	DrawFunc3D::DrawShadowFallModel(shadowMap, lightCamera, floor, trans, debugCam);
	//DrawFunc3D::DrawADSShadingModel(ligMgr, floor, trans, debugCam);
	DrawFunc3D::DrawADSShadingModel(ligMgr, testModel, debugCam);
}

void GameScene::OnImguiDebug()
{
}

void GameScene::OnFinalize()
{
}
