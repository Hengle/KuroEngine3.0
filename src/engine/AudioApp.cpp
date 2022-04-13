#include "AudioApp.h"

AudioApp* AudioApp::INSTANCE = nullptr;

using namespace std;

AudioApp::AudioApp()
{

	if (INSTANCE != nullptr)
	{
		printf("既にインスタンスがあります。AudioAppは１つのインスタンスしか持てません\n");
		assert(0);
	}
	INSTANCE = this;

	HRESULT result;

	//XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	//マスターボイスを生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
}

AudioApp::~AudioApp()
{
	xAudio2.Reset();
	for (auto itr = audios.begin(); itr != audios.end(); ++itr)
	{
		itr->Unload();
	}
}

void AudioApp::Update()
{
	for (auto& audio : audios)
	{
		audio.playTrigger = false;
	}
}

bool AudioApp::NowPlay(const int& Handle)
{
	auto itr = audios.begin();
	for (int i = 0; i < Handle; i++)
	{
		itr++;
	}

	if (itr->pSourceVoice != nullptr)
	{
		XAUDIO2_VOICE_STATE state;
		itr->pSourceVoice->GetState(&state);
		return !(state.pCurrentBufferContext == nullptr);
	}
	else return false;
}

void AudioApp::ChangeVolume(const int& Handle, float Volume)
{
	auto itr = audios.begin();
	for (int i = 0; i < Handle; i++)
	{
		itr++;
	}
	itr->volume = Volume;
	if (itr->pSourceVoice != nullptr)
	{
		itr->pSourceVoice->SetVolume(itr->volume);
	}
}

float AudioApp::GetVolume(const int& Handle)
{
	auto itr = audios.begin();
	for (int i = 0; i < Handle; i++)
	{
		itr++;
	}
	return itr->volume;
}

int AudioApp::LoadAudio(string FileName)
{
	//if (!audios.empty())
	//{
	//	int i = 0;
	//	for (auto itr = audios.begin(); itr != audios.end(); itr++)
	//	{
	//		if (itr->filePass == FileName)
	//		{
	//			return i;
	//		}
	//		i++;
	//	}
	//}

	//audios.emplace_back(FileName);

	//HRESULT result;

	////①ファイルオープン
	//	//ファイル入力ストリームのインスタンス
	//std::ifstream file;
	////.wavファイルをバイナリモードで開く
	//file.open(FileName.c_str(), std::ios_base::binary);
	////ファイルオープン失敗を検出する
	//if (file.fail())
	//{
	//	assert(0);
	//}

	////②.wavデータ解析
	////RITFヘッダーの読み込み
	//file.read((char*)&audios.back().riff, sizeof(audios.back().riff));
	////ファイルがRIFFかチェック
	//if (strncmp(audios.back().riff.chunk.id, "RIFF", 4) != 0)
	//{
	//	assert(0);
	//}

	////Formatチャンクの読み込み
	//file.read((char*)&audios.back().format, sizeof(audios.back().format));
	////ファイルがFormatかチェック
	//if (strncmp(audios.back().format.chunk.id, "fmt", 3) != 0)
	//{
	//	assert(0);
	//}

	////Dataチャンクの読み込み
	//file.read((char*)&audios.back().data, sizeof(audios.back().data));
	////ファイルがDataかチェック
	//if (strncmp(audios.back().data.id, "data", 4) != 0)
	//{
	//	assert(0);
	//}

	////Dataチャンクのデータ部（波形データ）の読み込み
	//audios.back().pBuffer = new char[audios.back().data.size];
	//file.read(audios.back().pBuffer, audios.back().data.size);

	////Waveファイルを閉じる
	//file.close();

	//return audios.size() - 1;
	if (!audios.empty())
	{
		int i = 0;
		for (auto itr = audios.begin(); itr != audios.end(); itr++)
		{
			if (itr->filePass == FileName)
			{
				return i;
			}
			i++;
		}
	}

	audios.emplace_back(FileName);

	HRESULT result;

	//①ファイルオープン
		//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(FileName.c_str(), std::ios_base::binary);
	//ファイルオープン失敗を検出する
	if (file.fail())
	{
		assert(0);
	}

	while (1)
	{
		Chunk chunk;
		file.read((char*)&chunk, sizeof(Chunk));

		if (strncmp(chunk.id, "RIFF", 4) == 0)
		{
			audios.back().riff.chunk = chunk;
			file.read(audios.back().riff.type, sizeof(audios.back().riff.type));
		}
		else if (strncmp(chunk.id, "fmt ", 4) == 0)
		{
			audios.back().format.chunk = chunk;
			file.read((char*)&audios.back().format.fmt, chunk.size);
		}
		else if (strncmp(chunk.id, "data", 4) == 0)
		{
			audios.back().data = chunk;
			//Dataチャンクのデータ部（波形データ）の読み込み
			audios.back().pBuffer = new char[audios.back().data.size];
			file.read(audios.back().pBuffer, audios.back().data.size);
			break;
		}
		else
		{
			//不必要な情報なら無視
			file.seekg(chunk.size, ios::cur);
		}
	}

	//Waveファイルを閉じる
	file.close();

	return audios.size() - 1;
}

int AudioApp::PlayWave(const int& Handle, bool LoopFlag)
{
	HRESULT result;

	auto itr = audios.begin();
	for (int i = 0; i < Handle; i++)
	{
		itr++;
	}

	//同時に同じ音源を再生しない
	if (itr->playTrigger)
	{
		return -1;
	}

	//ループ再生で再生しようとしたら既に流れているものを停止
	if (LoopFlag && NowPlay(Handle))
	{
		itr->pSourceVoice->Stop();
	}

//③サウンド再生
	WAVEFORMATEX wfex{};
	//波形フォーマットの設定
	memcpy(&wfex, &itr->format.fmt, sizeof(itr->format.fmt));
	wfex.wBitsPerSample = itr->format.fmt.nBlockAlign * 8 / itr->format.fmt.nChannels;

	//波形フォーマットを元にSourceVoiceの生成
	result = xAudio2->CreateSourceVoice(&itr->pSourceVoice, &wfex, 0, 2.0f, &voiceCallback);
	if FAILED(result)
	{
		return -1;
	}
	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)itr->pBuffer;
	buf.pContext = itr->pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = itr->data.size;
	if (LoopFlag)
	{
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	
	//音量の設定
	itr->pSourceVoice->SetVolume(itr->volume);

	//波形データの再生
	result = itr->pSourceVoice->SubmitSourceBuffer(&buf);
	result = itr->pSourceVoice->Start();

	itr->playTrigger = true;
}

void AudioApp::StopWave(const int& Handle)
{
	auto itr = audios.begin();
	for (int i = 0; i < Handle; i++)
	{
		itr++;
	}
	if (itr->pSourceVoice != nullptr)
	{
		itr->pSourceVoice->Stop();
		itr->pSourceVoice->FlushSourceBuffers();
	}
}

void AudioApp::AudioData::Unload()
{
	if (pBuffer != nullptr)delete[] pBuffer;
}
