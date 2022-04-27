#pragma once
#include"Color.h"
#include"D3D12Data.h"
#include<vector>
#include"Transform.h"
#include<memory>
class Model;
#include"Camera.h"

class LightManager;

static class DrawFunc3D
{
	//DrawLine
	static int DRAW_LINE_COUNT;
	//DrawNonShadingModel
	static int DRAW_NON_SHADING_COUNT;
	//DrawShadingModel
	static int DRAW_SHADING_COUNT;

public:
	//�Ăяo���J�E���g���Z�b�g
	static void CountReset()
	{
		DRAW_LINE_COUNT = 0;
		DRAW_NON_SHADING_COUNT = 0;
		DRAW_SHADING_COUNT = 0;
	}

	//���`��
	static void DrawLine(Camera& Cam, const Vec3<float>& From, const Vec3<float>& To, const Color& LineColor, const float& Thickness, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
	//�ʏ�`��
	static void DrawNonShadingModel(const std::weak_ptr<Model>Model, Transform& Transform, Camera& Camera, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
	//�e���`��
	static void DrawShadingModel(LightManager& LigManager, const std::weak_ptr<Model>Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
};