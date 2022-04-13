#pragma once
#include"Vec.h"
#include<stdio.h>

#include <dinput.h>
#define DIRECTINPUT_VERSION 0x0800	//DirectInputのバージョン指定
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include<wrl.h>
#include <Xinput.h>

//#include"Collision.h"
//namespace CollisionPrimitve
//{
//	class Ray;
//}

enum MOUSE_BUTTON { LEFT, RIGHT, CENTER };
enum XBOX_BUTTON {
	B = XINPUT_GAMEPAD_B,
	A = XINPUT_GAMEPAD_A,
	X = XINPUT_GAMEPAD_X,
	Y = XINPUT_GAMEPAD_Y,
	START = XINPUT_GAMEPAD_START,
	BACK = XINPUT_GAMEPAD_BACK,
	LB = XINPUT_GAMEPAD_LEFT_SHOULDER,
	RB = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
	DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
	DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
	DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,
	LT, RT
};
enum XBOX_STICK {
	L_UP, L_DOWN, L_LEFT, L_RIGHT,
	R_UP, R_DOWN, R_LEFT, R_RIGHT, XBOX_STICK_NUM
};

class UsersInput
{
private:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static UsersInput* INSTANCE;
public:
	static UsersInput* Instance()
	{
		if (INSTANCE == nullptr)
		{
			printf("UsersInputのインスタンスを呼び出そうとしましたがnullptrでした\n");
			assert(0);
		}
		return INSTANCE;
	}

	struct MouseMove
	{
		LONG IX;
		LONG IY;
		LONG IZ;
	};

private:
	ComPtr<IDirectInput8> dinput = nullptr;

	//キーボード
	ComPtr<IDirectInputDevice8> keyDev;
	static const int KEY_NUM = 256;
	BYTE keys[KEY_NUM] = {};
	BYTE oldkeys[KEY_NUM] = {};

	//マウス
	ComPtr<IDirectInputDevice8>mouseDev;
	DIMOUSESTATE2 mouseState = {};
	DIMOUSESTATE2 oldMouseState = {};
	//マウスのゲーム空間内でのレイ
	Vec2<float> mousePos;

	//XINPUT(コントローラー用)
	XINPUT_STATE xinputState;
	XINPUT_STATE oldXinputState;
	float shakePower = 0.0f;;
	int shakeTimer = 0;
	//デッドゾーンに入っているか(DeadRate : デッドゾーン判定の度合い、1.0fだとデフォルト)
	bool StickInDeadZone(Vec2<float>& Thumb, const Vec2<float>& DeadRate);

	void Initialize(const WNDCLASSEX& WinClass,const HWND& Hwnd);

public:
	UsersInput(const WNDCLASSEX& WinClass, const HWND& Hwnd)
	{
		if (INSTANCE != nullptr)
		{
			printf("既にインスタンスがあります。UsersInputは１つのインスタンスしか持てません\n");
			assert(0);
		}
		INSTANCE = this;
		Initialize(WinClass, Hwnd);
	}
	~UsersInput() {};

	void Update(const HWND& Hwnd, const Vec2<float>& WinSize);

	//キーボード
	bool OnTrigger(int KeyCode);
	bool Input(int KeyCode);
	bool OffTrigger(int KeyCode);

	//マウス
	bool OnTrigger(MOUSE_BUTTON Button);
	bool Input(MOUSE_BUTTON Button);
	bool OffTrigger(MOUSE_BUTTON Button);

	const Vec2<float>& GetMousePos()const { return mousePos; }
	MouseMove GetMouseMove();
	//Ray GetMouseRay();

	//XBOXコントローラー
	bool OnTrigger(XBOX_BUTTON Button);
	bool OnTrigger(XBOX_STICK StickInput, const float& DeadRange = 0.3f, const Vec2<float>& DeadRate = { 1.0f,1.0f });
	bool Input(XBOX_BUTTON Button);
	bool Input(XBOX_STICK StickInput, const float& DeadRange = 0.3f, const Vec2<float>& DeadRate = { 1.0f,1.0f });
	bool OffTrigger(XBOX_BUTTON Button);
	bool OffTrigger(XBOX_STICK StickInput, const float& DeadRange = 0.3f, const Vec2<float>& DeadRate = { 1.0f,1.0f });

	//デッドゾーン判定の度合い(1.0fだとデフォルト採用)
	Vec2<float>GetLeftStickVec(const Vec2<float>& DeadRate = { 1.0f,1.0f });
	Vec2<float>GetRightStickVec(const Vec2<float>& DeadRate = { 1.0f,1.0f });
	// "Power" must fit between 0.0f and 1.0f.
	void ShakeController(const float& Power, const int& Span);

	// 生のスティックのレートが欲しかったので臨時で実装しました。開発が終わったら消してください；；
	Vec2<float>GetLeftStickVecFuna(){return Vec2<float>(xinputState.Gamepad.sThumbLX, -xinputState.Gamepad.sThumbLY);}
	Vec2<float>GetRightStickVecFuna(){return Vec2<float>(xinputState.Gamepad.sThumbRX, -xinputState.Gamepad.sThumbRY);}

};

