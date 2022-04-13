#pragma once
#include"PostEffect.h"

class ComputePipeline;
class ConstantBuffer;
class TextureBuffer;
class RenderTarget;

class GaussianBlur : public PostEffect
{
	static const int NUM_WEIGHTS = 8;
	//重みテーブル
	float weights[NUM_WEIGHTS];

	//重みテーブルの定数バッファ
	std::shared_ptr<ConstantBuffer>weightConstBuff;
	//テクスチャ情報の定数バッファ
	std::shared_ptr<ConstantBuffer>texInfoConstBuff;

	//横ブラーの結果
	std::shared_ptr<ComputePipeline>xBlurPipeline;
	std::shared_ptr<TextureBuffer>xBlurResult;

	//縦ブラーの結果
	std::shared_ptr<ComputePipeline>yBlurPipeline;
	std::shared_ptr<TextureBuffer>yBlurResult;

	//最終結果
	std::shared_ptr<ComputePipeline>finalPipeline;
	std::shared_ptr<TextureBuffer>finalResult;

public:
	//ソース画像 & 結果描画先設定、ぼかし強さ
	GaussianBlur(const Vec2<int>& Size, const DXGI_FORMAT& Format, const float& BlurPower = 8.0f);
	//ボケ具合
	void SetBlurPower(const float& BlurPower);
	//実行
	void Excute(const ComPtr<ID3D12GraphicsCommandList>& CmdList, const std::shared_ptr<TextureBuffer>& SourceTex)override;

	//結果のテクスチャ取得
	std::shared_ptr<TextureBuffer>& GetResultTex() { return finalResult; }

	//ウィンドウサイズで結果を描画
	void DrawResult(const AlphaBlendMode& AlphaBlend);
};