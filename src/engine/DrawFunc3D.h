#pragma once
#include"Color.h"
#include"D3D12Data.h"
#include<vector>
#include"Transform.h"
#include<memory>
#include"Camera.h"

class Model;
class LightManager;

static class DrawFunc3D
{
	//DrawLine
	static int DRAW_LINE_COUNT;
	//DrawNonShadingModel
	static int DRAW_NON_SHADING_COUNT;
	//DrawADSShadingModel
	static int DRAW_ADS_SHADING_COUNT;
	//DrawPBRShadingModel
	static int DRAW_PBR_SHADING_COUNT;
	//DrawToonModel
	static int DRAW_TOON_COUNT;

public:
	//�Ăяo���J�E���g���Z�b�g
	static void CountReset()
	{
		DRAW_LINE_COUNT = 0;
		DRAW_NON_SHADING_COUNT = 0;
		DRAW_ADS_SHADING_COUNT = 0;
		DRAW_PBR_SHADING_COUNT = 0;
		DRAW_TOON_COUNT = 0;
	}

	//���`��
	static void DrawLine(Camera& Cam, const Vec3<float>& From, const Vec3<float>& To, const Color& LineColor, const float& Thickness, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
	//�ʏ�`��
	static void DrawNonShadingModel(const std::weak_ptr<Model>Model, Transform& Transform, Camera& Camera, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
	//�e���`��
	static void DrawADSShadingModel(LightManager& LigManager, const std::weak_ptr<Model>Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
	//�e���`��(PBR)
	static void DrawPBRShadingModel(LightManager& LigManager, const std::weak_ptr<Model>Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
	//�g�D�[���V�F�[�f�B���O
	static void DrawToonModel(const std::weak_ptr<TextureBuffer>ToonTex, LightManager& LigManager, const std::weak_ptr<Model>Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
};