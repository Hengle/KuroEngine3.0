#include "GameScene.h"
#include"KuroEngine.h"
#include"Importer.h"
#include"DrawFunc3D.h"
#include"Camera.h"
#include"Model.h"

GameScene::GameScene()
{
	std::array<std::shared_ptr<Model>, MODEL_NUM>loadModels =
	{
		Importer::Instance()->LoadGLTFModel("resource/user/gltf/", "monkey.glb"),
		 Importer::Instance()->LoadFBXModel("resource/user/", "Dragon.FBX"),
		 Importer::Instance()->LoadGLTFModel("resource/user/gltf/", "player.glb"),
		 Importer::Instance()->LoadFBXModel("resource/user/gltf/metalball/", "metalball.fbx"),
		 Importer::Instance()->LoadFBXModel("resource/user/gltf/woodball/", "woodcube.fbx"),
	};
	models = std::move(loadModels);
	models[METAL_BALL_FBX]->meshes[0].material->texBuff[NORMAL_TEX] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/gltf/metalball/MetalStainlessSteelBrushedElongated005_NRM_3K_METALNESS.jpg");
	models[METAL_BALL_FBX]->meshes[0].material->texBuff[METALNESS_TEX] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/gltf/metalball/MetalStainlessSteelBrushedElongated005_METALNESS_3K_METALNESS.jpg");
	models[METAL_BALL_FBX]->meshes[0].material->texBuff[ROUGHNESS_TEX] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/gltf/metalball/MetalStainlessSteelBrushedElongated005_ROUGHNESS_3K_METALNESS.jpg");

	models[WOOD_CUBE_FBX]->meshes[0].material->texBuff[NORMAL_TEX] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/gltf/woodball/WoodFineDark004_NRM_3K.jpg");

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

	//モデル切り替え
	if (0 < nowModel && UsersInput::Instance()->KeyOnTrigger(DIK_LEFT))
	{
		nowModel = (MODEL)(nowModel - 1);
	}
	if (nowModel < MODEL_NUM - 1 && UsersInput::Instance()->KeyOnTrigger(DIK_RIGHT))
	{
		nowModel = (MODEL)(nowModel + 1);
	}

	debugCam.Move();
}


void GameScene::OnDraw()
{
	static auto toonTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/toon.png");

	static std::shared_ptr<DepthStencil>dsv = D3D12App::Instance()->GenerateDepthStencil(
		D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize());

	dsv->Clear(D3D12App::Instance()->GetCmdList());

	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget() }, dsv);

	static const enum DRAW_MODE { ADS, TOON, PBR, MODE_NUM };
	static DRAW_MODE mode = ADS;
	if (mode == PBR)
	{
		DrawFunc3D::DrawPBRShadingModel(ligMgr, models[nowModel], trans, debugCam);
	}
	else if(mode == TOON)
	{
		DrawFunc3D::DrawToonModel(toonTex, ligMgr, models[nowModel], trans, debugCam);
	}
	else
	{
		DrawFunc3D::DrawADSShadingModel(ligMgr, models[nowModel], trans, debugCam);
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
	ImguiApp::Instance()->DebugMaterial(models[nowModel]->meshes[0].material, REFERENCE);
	ImguiApp::Instance()->DebugMaterial(models[nowModel]->meshes[0].material, REWRITE);
}

void GameScene::OnFinalize()
{
}
