#pragma once
#include"D3D12Data.h"
#include<vector>
#include"Transform.h"
#include<memory>
class Model;
#include"Camera.h"

class LightManager;

static class DrawFunc3D
{
	//DrawNonShadingModel
	static int DRAW_NON_SHADING_COUNT;
	//DrawShadingModel
	static int DRAW_SHADING_COUNT;

public:
	//�Ăяo���J�E���g���Z�b�g
	static void CountReset()
	{
		DRAW_NON_SHADING_COUNT = 0;
		DRAW_SHADING_COUNT = 0;
	}

	//�ʏ�`��
	static void DrawNonShadingModel(const std::weak_ptr<Model>Model, Transform& Transform, Camera& Camera, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
	//�e���`��
	static void DrawShadingModel(LightManager& LigManager, const std::weak_ptr<Model>Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
};