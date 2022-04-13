#pragma once
#include <Windows.h>
#include <tchar.h>
#include <sstream>

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
class Fps
{
	const int frameRate;
	const float minFrameTime;
	float frameTime = 0;
	LARGE_INTEGER timeStart;
	LARGE_INTEGER timeEnd;
	LARGE_INTEGER timeFreq;
	// fpsを取得するなら0で初期化しないとゴミが混ざってマイナスから始まったりする(かも知れない)
	float fps = 0;

public:
	Fps(const int& FrameRate);
	void Update();

	const float& GetNowFps()const { return fps; }
};