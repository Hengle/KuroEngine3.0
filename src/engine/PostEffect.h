#pragma once
#include<wrl.h>
#include<d3d12.h>
#include<memory>
#include"Sprite.h"

class TextureBuffer;

class PostEffect
{
	static std::shared_ptr<Sprite>WIN_SIZE_SPRITE;
protected:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static std::shared_ptr<Sprite>GetWinSizeSprite();

public:
	virtual void Excute(const ComPtr<ID3D12GraphicsCommandList>& CmdList, const std::shared_ptr<TextureBuffer>& SourceTex) = 0;
	//グラフィックマネージャに登録
	void Register(const std::shared_ptr<TextureBuffer>& SourceTex);
};