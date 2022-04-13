#include "PostEffect.h"
#include"WinApp.h"
std::shared_ptr<Sprite>PostEffect::WIN_SIZE_SPRITE;

std::shared_ptr<Sprite> PostEffect::GetWinSizeSprite()
{
	if (!WIN_SIZE_SPRITE)
	{
		WIN_SIZE_SPRITE = std::make_shared<Sprite>(nullptr, "PostEffect's Sprite - WinSize");
		WIN_SIZE_SPRITE->mesh.SetSize(WinApp::Instance()->GetExpandWinSize());
	}
	return WIN_SIZE_SPRITE;
}

#include"KuroEngine.h"
void PostEffect::Register(const std::shared_ptr<TextureBuffer>& SourceTex)
{
	KuroEngine::Instance().Graphics().ExcutePostEffect(this, SourceTex);
}
