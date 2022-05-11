#include "GameScene.h"
#include"KuroEngine.h"
#include"Importer.h"
#include"DrawFunc3D.h"
#include"Camera.h"
#include"Model.h"

GameScene::GameScene()
{
	//dirLig.SetDir(Vec3<Angle>(50, -30, 0));
	ligMgr.RegisterDirLight(&dirLig);
	ligMgr.RegisterPointLight(&ptLig);

	playerModel = Importer::Instance()->LoadFBXModel("resource/user/player/", "player.fbx");
	playerModel->MeshSmoothing();

	drawTest = Importer::Instance()->LoadGLTFModel("resource/user/gltf/", "monkey.glb");
	//drawTest = Importer::Instance()->LoadFBXModel("resource/user/", "Dragon.FBX");
	drawTest->MeshSmoothing();

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

	if (UsersInput::Instance()->KeyOnTrigger(DIK_1))
	{
		dirLig.SetActive();
	}
	if (UsersInput::Instance()->KeyOnTrigger(DIK_2))
	{
		ptLig.SetActive();
	}

	debugCam.Move();
}


void GameScene::OnDraw()
{
	static auto testglTF0 = Importer::Instance()->LoadGLTFModel("resource/user/gltf/", "player.glb");
	static auto toonTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/toon.png");

	static std::shared_ptr<DepthStencil>dsv = D3D12App::Instance()->GenerateDepthStencil(
		D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize());

	dsv->Clear(D3D12App::Instance()->GetCmdList());

	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() }, dsv);

	static const enum DRAW_MODE { ADS, TOON, PBR, MODE_NUM };
	static DRAW_MODE mode = ADS;
	if (mode == PBR)
	{
		DrawFunc3D::DrawPBRShadingModel(ligMgr, drawTest, trans, debugCam);
	}
	else if(mode == TOON)
	{
		DrawFunc3D::DrawToonModel(toonTex, ligMgr, drawTest, trans, debugCam);
	}
	else
	{
		DrawFunc3D::DrawADSShadingModel(ligMgr, drawTest, trans, debugCam);
	}

	if (UsersInput::Instance()->KeyOnTrigger(DIK_R))
	{
		mode = (DRAW_MODE)(mode + 1);
		if (mode == MODE_NUM)mode = ADS;
	}

	//DrawFunc3D::DrawLine(debugCam, { 0,0,0 }, ptLig.GetPos(), Color(1.0f, 0.0f, 0.0f, 1.0f), 0.3f);
}

void GameScene::OnImguiDebug()
{
	ImguiApp::Instance()->DebugMaterial(drawTest->meshes[0].material, REFERENCE);
	ImguiApp::Instance()->DebugMaterial(drawTest->meshes[0].material, REWRITE);
}

void GameScene::OnFinalize()
{
}
