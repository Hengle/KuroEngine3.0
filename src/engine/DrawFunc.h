#pragma once
#include"D3D12Data.h"
#include<memory>
#include<vector>
#include"Vec.h"
#include"Color.h"
#include<map>

static class DrawFunc
{
	//DrawLine
	static std::shared_ptr<GraphicsPipeline>LINE_PIPELINE[AlphaBlendModeNum];
	static int DRAW_LINE_COUNT;	//呼ばれた回数
	static std::vector<std::shared_ptr<VertexBuffer>>LINE_VERTEX_BUFF;	//DrawLine用

	//DrawBox
	static std::map<DXGI_FORMAT, std::map<AlphaBlendMode, std::shared_ptr<GraphicsPipeline>>>BOX_PIPELINE;
	static int DRAW_BOX_COUNT;
	static std::vector<std::shared_ptr<VertexBuffer>>BOX_VERTEX_BUFF;

	//DrawCircle
	static std::shared_ptr<GraphicsPipeline>CIRCLE_PIPELINE[AlphaBlendModeNum];
	static int DRAW_CIRCLE_COUNT;
	static std::vector<std::shared_ptr<VertexBuffer>>CIRCLE_VERTEX_BUFF;

	//DrawExtendGraph
	static std::shared_ptr<GraphicsPipeline>EXTEND_GRAPH_PIPELINE[AlphaBlendModeNum];
	static int DRAW_EXTEND_GRAPH_COUNT;
	static std::vector<std::shared_ptr<VertexBuffer>>EXTEND_GRAPH_VERTEX_BUFF;

	//DrawRotaGraph
	static std::shared_ptr<GraphicsPipeline>ROTA_GRAPH_PIPELINE[AlphaBlendModeNum];
	static int DRAW_ROTA_GRAPH_COUNT;
	static std::vector<std::shared_ptr<VertexBuffer>>ROTA_GRAPH_VERTEX_BUFF;

public:
	//呼び出しカウントリセット
	static void CountReset()
	{
		DRAW_LINE_COUNT = 0;
		DRAW_BOX_COUNT = 0;
		DRAW_CIRCLE_COUNT = 0;
		DRAW_EXTEND_GRAPH_COUNT = 0;
		DRAW_ROTA_GRAPH_COUNT = 0;
	}

	/// <summary>
	/// 直線の描画
	/// </summary>
	/// <param name="FromPos">起点座標</param>
	/// <param name="ToPos">終点座標</param>
	/// <param name="Color">色</param>
	/// <param name="BlendMode">ブレンドモード</param>
	static void DrawLine2D(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const Color& LineColor, const AlphaBlendMode& BlendMode = AlphaBlendMode_None);

	/// <summary>
	/// 直線の描画（画像）
	/// </summary>
	/// <param name="FromPos">起点座標</param>
	/// <param name="ToPos">終点座標</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="Thickness">線の太さ</param>
	/// <param name="BlendMode">ブレンドモード</param>
	/// <param name="Miror">反転フラグ</param>
	static void DrawLine2DGraph(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const std::shared_ptr<TextureBuffer>& Tex, const int& Thickness, const AlphaBlendMode& BlendMode = AlphaBlendMode_None, const Vec2<bool>& Mirror = { false,false });

	/// <summary>
	/// 2D四角形の描画
	/// </summary>
	/// <param name="LeftUpPos">左上座標</param>
	/// <param name="RightBottomPos">右下座標</param>
	/// <param name="Color">色</param>
	/// <param name="FillFlg">中を塗りつぶすか</param>
	/// <param name="BlendMode">ブレンドモード</param>
	static void DrawBox2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Color& BoxColor, const DXGI_FORMAT& Format, const bool& FillFlg = false, const AlphaBlendMode& BlendMode = AlphaBlendMode_None);

	/// <summary>
	/// ２D円の描画
	/// </summary>
	/// <param name="Center">中心座標</param>
	/// <param name="Radius">半径</param>
	/// <param name="Color">色</param>
	/// <param name="FillFlg">中を塗りつぶすか</param>
	/// <param name="LineThickness">中を塗りつぶさない場合の線の太さ</param>
	/// <param name="BlendMode">ブレンドモード</param>
	static void DrawCircle2D(const Vec2<float>& Center, const float& Radius, const Color& CircleColor, const bool& FillFlg = false, const int& LineThickness = 1, const AlphaBlendMode& BlendMode = AlphaBlendMode_None);

	/// <summary>
	/// 画像描画
	/// </summary>
	/// <param name="LeftUpPos">矩形の左上座標</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="BlendMode">ブレンドモード</param>
	static void DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& Tex, const AlphaBlendMode& BlendMode = AlphaBlendMode_None, const Vec2<bool>& Miror = { false,false });

	/// <summary>
	/// 拡大縮小描画
	/// </summary>
	/// <param name="LeftUpPos">矩形の左上座標</param>
	/// <param name="RightBottomPos">矩形の右上座標</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="BlendMode">ブレンドモード</param>
	static void DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos,
		const std::shared_ptr<TextureBuffer>& Tex, const AlphaBlendMode& BlendMode = AlphaBlendMode_None, const Vec2<bool>& Miror = { false,false });

	/// <summary>
	/// ２D画像回転描画
	/// </summary>
	/// <param name="Center">中心座標</param>
	/// <param name="ExtRate">拡大率</param>
	/// <param name="Radian">回転角度</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="BlendMode">ブレンドモード</param>
	/// <param name="LRTurn">左右反転フラグ</param>
	static void DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
		const std::shared_ptr<TextureBuffer>& Tex, const Vec2<float>& RotaCenterUV = { 0.5f,0.5f },
		const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans, const Vec2<bool>& Mirror = { false,false });
};