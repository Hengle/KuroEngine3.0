#pragma once
#include<memory>
class GraphicsPipeline;
class ConstantBuffer;
class LightManager;
class TextureBuffer;

#include"Color.h"
#include"KuroMath.h"
#include"SpriteMesh.h"
#include"Transform2D.h"

class Sprite_Shadow
{
private:
	static std::shared_ptr<GraphicsPipeline>PIPELINE_TRANS;
	//視点座標用の定数バッファ
	static std::shared_ptr<ConstantBuffer>EYE_POS_BUFF;
	//白のベタ塗りテクスチャ
	static std::shared_ptr<TextureBuffer>DEFAULT_TEX;
	// (0,0,-1) のベタ塗りノーマルマップ
	static std::shared_ptr<TextureBuffer>DEFAULT_NORMAL_MAP;
	//黒のベタ塗りテクスチャ
	static std::shared_ptr<TextureBuffer>DEFAULT_EMISSIVE_MAP;

public:
	static void SetEyePos(Vec3<float> EyePos);

private:
	//定数バッファ送信用データ
	struct ConstantData
	{
		Matrix mat;
		Color color;
		float z = 0.0f;
		float diffuse = 1.0f;	//ディヒューズの影響度
		float specular = 1.0f;	//スペキュラーの影響度
		float lim = 1.0f;	//リムライトの影響度
	}constData;

	//定数バッファ
	std::shared_ptr<ConstantBuffer>constBuff;

	//テクスチャバッファ
	std::shared_ptr<TextureBuffer>texBuff;
	//ノーマルマップ
	std::shared_ptr<TextureBuffer>normalMap;
	//エミッシブマップ
	std::shared_ptr<TextureBuffer>emissiveMap;

public:
	//トランスフォーム
	Transform2D transform;
	//メッシュ（頂点情報）
	SpriteMesh mesh;

	//テクスチャ、ノーマルマップ、スプライト名
	Sprite_Shadow(const std::shared_ptr<TextureBuffer>& Texture = nullptr, const std::shared_ptr<TextureBuffer>& NormalMap = nullptr,
		const std::shared_ptr<TextureBuffer>& EmissiveMap = nullptr, const char* Name = nullptr);

	//テクスチャとノーマルマップセット
	void SetTexture(const std::shared_ptr<TextureBuffer>& Texture, const std::shared_ptr<TextureBuffer>& NormalMap, const std::shared_ptr<TextureBuffer>& EmissiveMap);

	//色セット（描画の際にこの値が乗算される）
	void SetColor(const Color& Color);

	//陰影をつける際に使う深度値
	void SetDepth(const float& Depth);

	//ライトの影響度設定
	void SetDiffuseAffect(const float& Diffuse);
	void SetSpecularAffect(const float& Specular);
	void SetLimAffect(const float& Lim);

	//描画
	void Draw(LightManager& LigManager);
};

