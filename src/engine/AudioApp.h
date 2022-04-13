#pragma once
#include<xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include<wrl.h>

#include<fstream>
#include<assert.h>

#include<list>

#include<string>

class AudioApp
{
private:
	static AudioApp* INSTANCE;
public:
	static AudioApp* Instance()
	{
		if (INSTANCE == nullptr)
		{
			printf("AudioAppのインスタンスを呼び出そうとしましたがnullptrでした\n");
			assert(0);
		}
		return INSTANCE;
	}
private:
	//チャンクヘッダ
	struct Chunk
	{
		char id[4];		//チャンク毎のID
		int32_t size;	//チャンクのサイズ
	};

	//RTFFヘッダチャンク
	struct RiffHeader
	{
		Chunk chunk;	//"RIFF"
		char type[4];	//"WAVE"
	};

	//FMTチャンク
	struct FormatChunck
	{
		Chunk chunk;	//"fmt "
		WAVEFORMAT fmt;	//波形フォーマット
	};

	Microsoft::WRL::ComPtr<IXAudio2>xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	class XAudio2VoiceCallback : public IXAudio2VoiceCallback
	{
	public:
		virtual ~XAudio2VoiceCallback() {};
		//ボイス処理パスの開始時
		STDMETHOD_(void, OnVoiceProcessingPassStart)(THIS_ UINT32 BytesRequired) {};
		//ボイス処理パスの終了時
		STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) {};
		//バッファストリームの再生が終了したとき
		STDMETHOD_(void, OnStreamEnd) (THIS) {};
		//バッファの使用開始時
		STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) {};
		//バッファの末尾に達した時
		STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext){	};
		//再生がループ位置に達した時
		STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) {};
		//ボイスの実行エラー時
		STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) {};
	};
	XAudio2VoiceCallback voiceCallback;

	class AudioData
	{
	public:
		std::string filePass;
		RiffHeader riff;
		FormatChunck format;
		Chunk data;
		char* pBuffer = nullptr;
		IXAudio2SourceVoice* pSourceVoice = nullptr;
		float volume = 1.0f;
		bool loop = false;
		bool playTrigger = false;

		AudioData(std::string FilePass)
			:filePass(FilePass) {};
		void Unload();
	};

	std::list<AudioData>audios;

public:
	AudioApp();
	~AudioApp();

	void Update();
	bool NowPlay(const int& Handle);
	void ChangeVolume(const int& Handle, float Volume);
	float GetVolume(const int& Handle);
	int LoadAudio(std::string FileName);
	int PlayWave(const int& Handle, bool LoopFlag = false);
	void StopWave(const int& Handle);
};