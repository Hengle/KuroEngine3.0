#pragma once
#include"D3D12Data.h"
#include<vector>
#include"Transform.h"
#include<memory>
class Model;
#include"Camera.h"

static class DrawFunc3D
{
	//DrawModel
	//static std::shared_ptr<GraphicsPipeline>DEFAULT_PIPELINE[AlphaBlendModeNum];
	static int DRAW_NON_SHADING_COUNT;
	//static std::vector<std::shared_ptr<ConstantBuffer>>DEFAULT_TRANSFORM_BUFF;

public:
	//呼び出しカウントリセット
	static void CountReset()
	{
		DRAW_NON_SHADING_COUNT = 0;
	}

	//通常描画
	static void DrawNonShadingModel(const std::weak_ptr<Model>Model, Transform& Transform, Camera& Camera, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);
};