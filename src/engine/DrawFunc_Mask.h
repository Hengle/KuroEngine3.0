#pragma once
#include"D3D12Data.h"
#include<memory>
#include<vector>
#include"Vec.h"

//描画範囲をマスク（範囲外は指定したアルファ値で描画）
class DrawFunc_Mask
{
	//DrawExtendGraph
	class ExtendGraphVertex
	{
	public:
		Vec2<float>leftUpPos;
		Vec2<float>rightBottomPos;
		Vec2<float>maskLeftUpPos;
		Vec2<float>maskRightBottomPos;
		Vec2<int> miror;
		float maskAlpha;	//範囲外の描画アルファ値
		ExtendGraphVertex(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos, const Vec2<bool>& Miror, const float& MaskAlpha)
			:leftUpPos(LeftUpPos), rightBottomPos(RightBottomPos), maskLeftUpPos(MaskLeftUpPos), maskRightBottomPos(MaskRightBottomPos), miror({ Miror.x ? 1 : 0 ,Miror.y ? 1 : 0 }), maskAlpha(MaskAlpha) {}
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
	/// <param name="Tex">テクスチャ</param>
	/// <param name="MaskLeftUpPos">マスク範囲の左上座標</param>
	/// <param name="MaskRightBottomPos">マスク範囲の右下座標</param>
	/// <param name="Miror">反転フラグ</param>
	/// <param name="MaskAlpha">マスク範囲外のアルファ値</param>
	static void DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& Tex,
		const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos,
		const Vec2<bool>& Miror = { false,false }, const float& MaskAlpha = 0.0f);

	/// <summary>
	///  拡大縮小描画
	/// </summary>
	/// <param name="LeftUpPos">矩形の左上座標</param>
	/// <param name="RightBottomPos">矩形の右下座標</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="MaskLeftUpPos">マスク範囲の左上座標</param>
	/// <param name="MaskRightBottomPos">マスク範囲の右下座標</param>
	/// <param name="Miror">反転フラグ</param>
	/// <param name="MaskAlpha">マスク範囲外のアルファ値</param>
	static void DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const std::shared_ptr<TextureBuffer>& Tex,
		const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos,
		const Vec2<bool>& Miror = { false,false }, const float& MaskAlpha = 0.0f);

	/// <summary>
	/// ２D画像回転描画
	/// </summary>
	/// <param name="Center">中心座標</param>
	/// <param name="ExtRate">拡大率</param>
	/// <param name="Radian">回転角度</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="MaskCenterPos">マスク範囲中心座標</param>
	/// <param name="MaskSize">マスク範囲サイズ</param>
	/// <param name="RotaCenterUV">回転の中心UV</param>
	/// <param name="Miror">反転フラグ</param>
	/// <param name="MaskAlpha">マスク範囲外のアルファ値</param>
	static void DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian, const std::shared_ptr<TextureBuffer>& Tex,
		const Vec2<float>& MaskCenterPos, const Vec2<float>& MaskSize,
		const Vec2<float>& RotaCenterUV = { 0.5f,0.5f }, const Vec2<bool>& Miror = { false,false }, const float& MaskAlpha = 0.0f);

	/// <summary>
	/// 直線の描画（画像）
	/// </summary>
	/// <param name="FromPos">起点座標</param>
	/// <param name="ToPos">終点座標</param>
	/// <param name="Tex">テクスチャ</param>
	/// <param name="Thickness">線の太さ</param>
	/// <param name="MaskLeftUpPos">マスク範囲の左上座標</param>
	/// <param name="MaskRightBottomPos">マスク範囲の右下座標</param>
	/// <param name="BlendMode">ブレンドモード</param>
	/// <param name="Miror">反転フラグ</param>
	static void DrawLine2DGraph(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const std::shared_ptr<TextureBuffer>& Tex, const int& Thickness,
		const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos, const Vec2<bool>& Mirror = { false,false });

};

