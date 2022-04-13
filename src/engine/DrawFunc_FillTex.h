#pragma once
#include"D3D12Data.h"
#include<memory>
#include<vector>
#include"Vec.h"
#include"Color.h"

//画像にクリッピング、他の画像で塗る
class DrawFunc_FillTex
{
	//DrawExtendGraph
	class ExtendGraphVertex
	{
	public:
		Vec2<float>leftUpPos;
		Vec2<float>rightBottomPos;
		float alpha;
		Vec2<int> miror;
		Vec2<float>leftUpPaintUV;
		Vec2<float>rightBottomPaintUV;
		ExtendGraphVertex(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const float& Alpha, const Vec2<bool>& Miror, const Vec2<float>& LeftUpPaintUV, const Vec2<float>& RightBottomPaintUV)
			:leftUpPos(LeftUpPos), rightBottomPos(RightBottomPos), alpha(Alpha), miror({ Miror.x ? 1 : 0 ,Miror.y ? 1 : 0 }), leftUpPaintUV(LeftUpPaintUV), rightBottomPaintUV(RightBottomPaintUV) {}
	};
	static std::shared_ptr<GraphicsPipeline>EXTEND_GRAPH_PIPELINE;
	static int DRAW_EXTEND_GRAPH_COUNT;
	static std::vector<std::shared_ptr<VertexBuffer>>EXTEND_GRAPH_VERTEX_BUFF;
	static void CreateExtendGraphFunc();

	//DrawRotaGraph
	static std::shared_ptr<GraphicsPipeline>ROTA_GRAPH_PIPELINE;
	static int DRAW_ROTA_GRAPH_COUNT;
	static std::vector<std::shared_ptr<VertexBuffer>>ROTA_GRAPH_VERTEX_BUFF;

public:
	//呼び出しカウントリセット
	static void CountReset()
	{
		DRAW_EXTEND_GRAPH_COUNT = 0;
		DRAW_ROTA_GRAPH_COUNT = 0;
	}

	/// <summary>
	/// 画像描画
	/// </summary>
	/// <param name="LeftUpPos">矩形の左上座標</param>
	/// <param name="DestTex">テクスチャ</param>
	/// <param name="SrcTex">塗りつぶすテクスチャ</param>
	/// <param name="Miror">矩形の右上座標</param>
	/// <param name="LeftUpPaintUV">塗りつぶす範囲の左上UV</param>
	/// <param name="RightBottomPaintUV">塗りつぶす範囲の右下UV</param>
	static void DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& DestTex, const std::shared_ptr<TextureBuffer>& SrcTex,
		const float& SrcAlpha, const Vec2<bool>& Miror = { false,false }, const Vec2<float>& LeftUpPaintUV = { 0.0f,0.0f }, const Vec2<float>& RightBottomPaintUV = { 1.0f,1.0f });

	/// <summary>
	/// 拡大縮小描画
	/// </summary>
	/// <param name="LeftUpPos">矩形の左上座標</param>
	/// <param name="RightBottomPos">矩形の右上座標</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="Paint">塗りつぶす色</param>
	/// <param name="Miror">反転フラグ</param>
	/// <param name="LeftUpPaintUV">塗りつぶす範囲の左上UV</param>
	/// <param name="RightBottomPaintUV">塗りつぶす範囲の右下UV</param>
	static void DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos,
		const std::shared_ptr<TextureBuffer>& DestTex, const std::shared_ptr<TextureBuffer>& SrcTex, const float& SrcAlpha,
		const Vec2<bool>& Miror = { false,false },	const Vec2<float>& LeftUpPaintUV = { 0.0f,0.0f }, const Vec2<float>& RightBottomPaintUV = { 1.0f,1.0f });

	/// <summary>
	/// ２D画像回転描画
	/// </summary>
	/// <param name="Center">中心座標</param>
	/// <param name="ExtRate">拡大率</param>
	/// <param name="Radian">回転角度</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="Paint">塗りつぶす色</param>
	static void DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
		const std::shared_ptr<TextureBuffer>& DestTex, const std::shared_ptr<TextureBuffer>& SrcTex, const float& SrcAlpha, const Vec2<float>& RotaCenterUV = { 0.5f,0.5f },
		const Vec2<bool>& Miror = { false,false },const Vec2<float>& LeftUpPaintUV = { 0.0f,0.0f }, const Vec2<float>& RightBottomPaintUV = { 1.0f,1.0f });
};

