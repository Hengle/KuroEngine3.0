#include "UsersInput.h"
#include <cassert>
#include<DirectXMath.h>
#include"KuroFunc.h"

#pragma comment(lib,"xinput.lib")

#define STICK_INPUT_MAX 32768.0f

UsersInput* UsersInput::INSTANCE = nullptr;

bool UsersInput::StickInDeadZone(Vec2<float>& Thumb, const Vec2<float>& DeadRate)
{
	bool x = false, y = false;
	if ((Thumb.x < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * DeadRate.x
		&& Thumb.x > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * DeadRate.x))
	{
		Thumb.x = 0.0f;
		x = true;
	}
	if((Thumb.y < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * DeadRate.y
			&& Thumb.y > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * DeadRate.y))
	{
		Thumb.y = 0.0f;
		y = true;
	}
	if (x && y)return true;
	return false;
}

void UsersInput::Initialize(const WNDCLASSEX& WinClass, const HWND& Hwnd)
{
	HRESULT hr;
	//DirectInputオブジェクトの生成
	if (FAILED(hr = DirectInput8Create(
		WinClass.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr)))assert(0);

	//キーボードデバイスの生成
	if (FAILED(hr = dinput->CreateDevice(GUID_SysKeyboard, &keyDev, NULL)))assert(0);
	if (FAILED(hr = keyDev->SetDataFormat(&c_dfDIKeyboard)))assert(0);	//標準形式
	if (FAILED(hr = keyDev->SetCooperativeLevel(		//排他制御レベルのセット
		Hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE/* | DISCL_NOWINKEY*/)))assert(0);

	//マウスデバイスの生成
	if (FAILED(hr = dinput->CreateDevice(GUID_SysMouse, &mouseDev, NULL)))assert(0);
	if (FAILED(hr = mouseDev->SetDataFormat(&c_dfDIMouse2)))assert(0);
	if (FAILED(hr = mouseDev->SetCooperativeLevel(
		Hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY)))assert(0);

	/*
		使っているフラグについて
		DISCL_FOREGROUND	画面が手前にある場合のみ入力を受け付ける
		DISCL_NONEXCLUSIVE	デバイスをこのアプリだけで専有しない
		DISCL_NOWINKEY		Windowsキーを無効にする
	*/
}

void UsersInput::Update(const HWND& Hwnd, const Vec2<float>& WinSize)
{
	//キーボード
	memcpy(oldkeys, keys, KEY_NUM);
	//キーボード情報の取得開始
	HRESULT result = keyDev->Acquire();	//本当は一回でいいが、アプリが裏面から全面に戻る度呼び出す必要があるため、毎フレーム呼び出す。
	//全キーの入力状態を取得する
	result = keyDev->GetDeviceState(sizeof(keys), keys);

	//マウス
	result = mouseDev->Acquire();
	oldMouseState = mouseState;
	result = mouseDev->GetDeviceState(sizeof(mouseState), &mouseState);

	POINT p;
	GetCursorPos(&p);
	ScreenToClient(Hwnd, &p);
	mousePos.x = p.x;
	mousePos.x = KuroFunc::Clamp(0, WinSize.x, mousePos.x);
	mousePos.y = p.y;
	mousePos.y = KuroFunc::Clamp(0, WinSize.y, mousePos.y);

	//コントローラー
	for (int i = 0; i < CONTROLLER_NUM; ++i)
	{
		oldXinputState[i] = xinputState[i];
		ZeroMemory(&xinputState[i], sizeof(XINPUT_STATE));

		DWORD dwResult = XInputGetState(i, &xinputState[i]);

		//コントローラーが接続されていない
		if (dwResult != ERROR_SUCCESS)continue;

		//コントローラーが接続されている
		if (0 < shakeTimer[i])
		{
			shakeTimer[i]--;
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

			if (shakeTimer == 0)
			{
				vibration.wLeftMotorSpeed = 0.0f; // use any value between 0-65535 here
				vibration.wRightMotorSpeed = 0.0f; // use any value between 0-65535 here
			}
			else
			{
				vibration.wLeftMotorSpeed = 65535.0f * shakePower[i]; // use any value between 0-65535 here
				vibration.wRightMotorSpeed = 65535.0f * shakePower[i]; // use any value between 0-65535 here
			}
			XInputSetState(dwResult, &vibration);
		}
	}
}

bool UsersInput::KeyOnTrigger(int KeyCode)
{
	return (!oldkeys[KeyCode] && keys[KeyCode]);
}

bool UsersInput::KeyInput(int KeyCode)
{
	return keys[KeyCode];
}

bool UsersInput::KeyOffTrigger(int KeyCode)
{
	return (oldkeys[KeyCode] && !keys[KeyCode]);
}

bool UsersInput::MouseOnTrigger(MOUSE_BUTTON Button)
{
	return (!oldMouseState.rgbButtons[Button] && mouseState.rgbButtons[Button]);
}

bool UsersInput::MouseInput(MOUSE_BUTTON Button)
{
	return mouseState.rgbButtons[Button];
}

bool UsersInput::MouseOffTrigger(MOUSE_BUTTON Button)
{
	return (oldMouseState.rgbButtons[Button] && !mouseState.rgbButtons[Button]);
}

UsersInput::MouseMove UsersInput::GetMouseMove()
{
	MouseMove tmp;
	tmp.IX = mouseState.lX;
	tmp.IY = mouseState.lY;
	tmp.IZ = mouseState.lZ;
	return tmp;
}

//#include"Object.h"
//#include"WinApp.h"
//Ray UsersInput::GetMouseRay()
//{
//	Ray mouseRay;
//	Vec3<float> start = MyFunc::ConvertScreenToWorld(mousePos, 0.0f,
//	Camera::GetNowCam()->MatView(),
//		Camera::GetNowCam()->MatProjection(),
//		App::GetWinApp().GetWinSize());
//	mouseRay.start = { start.x,start.y,start.z };
//
//	Vec3<float> to = MyFunc::ConvertScreenToWorld(mousePos, 1.0f,
//		Camera::GetNowCam()->MatView(),
//		Camera::GetNowCam()->MatProjection(),
//		App::GetWinApp().GetWinSize());
//	Vec3<float> vec(to - start);
//	mouseRay.dir = vec.GetNormal();
//
//	return mouseRay;
//}

bool UsersInput::ControllerOnTrigger(const int& ControllerIdx, XBOX_BUTTON Button)
{
	//トリガー
	if (Button == LT)
	{
		return oldXinputState[ControllerIdx].Gamepad.bLeftTrigger <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD
			&& ControllerInput(ControllerIdx, Button);
	}
	else if (Button == RT)
	{
		return oldXinputState[ControllerIdx].Gamepad.bRightTrigger <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD
			&& ControllerInput(ControllerIdx, Button);
	}
	else
	{
		return !(oldXinputState[ControllerIdx].Gamepad.wButtons & Button)
			&& ControllerInput(ControllerIdx, Button);
	}
	assert(0);
	return false;
}

bool UsersInput::ControllerOnTrigger(const int& ControllerIdx, XBOX_STICK StickInput, const float& DeadRange, const Vec2<float>& DeadRate)
{
	Vec2<float>oldVec;
	Vec2<float>vec;
	bool isLeftStick = StickInput <= L_RIGHT;
	if (isLeftStick)
	{
		oldVec = Vec2<float>(oldXinputState[ControllerIdx].Gamepad.sThumbLX, oldXinputState[ControllerIdx].Gamepad.sThumbLY);
		vec = Vec2<float>(xinputState[ControllerIdx].Gamepad.sThumbLX, xinputState[ControllerIdx].Gamepad.sThumbLY);
	}
	else
	{
		oldVec = Vec2<float>(oldXinputState[ControllerIdx].Gamepad.sThumbRX, oldXinputState[ControllerIdx].Gamepad.sThumbRY);
		vec = Vec2<float>(xinputState[ControllerIdx].Gamepad.sThumbRX, xinputState[ControllerIdx].Gamepad.sThumbRY);
	}

	if (!StickInDeadZone(oldVec, DeadRate))return false;
	if (StickInDeadZone(vec, DeadRate))return false;

	bool result = false;

	if (StickInput % 4 == L_UP)
	{
		result = !(DeadRange < (oldVec.y / STICK_INPUT_MAX))
			&& DeadRange < (vec.y / STICK_INPUT_MAX);
	}
	else if (StickInput % 4 == L_DOWN)
	{
		result = !(oldVec.y / STICK_INPUT_MAX < -DeadRange)
			&& vec.y / STICK_INPUT_MAX < -DeadRange;
	}
	else if (StickInput % 4 == L_RIGHT)
	{
		result = !(DeadRange < (oldVec.x / STICK_INPUT_MAX))
			&& DeadRange < (vec.x / STICK_INPUT_MAX);
	}
	else if (StickInput % 4 == L_LEFT)
	{
		result =  !(oldVec.x / STICK_INPUT_MAX < -DeadRange)
			&& vec.x / STICK_INPUT_MAX < -DeadRange;
	}
	else
	{
		assert(0);
	}

	return result;
}

bool UsersInput::ControllerInput(const int& ControllerIdx, XBOX_BUTTON Button)
{
	if (Button == LT)
	{
		return XINPUT_GAMEPAD_TRIGGER_THRESHOLD < xinputState[ControllerIdx].Gamepad.bLeftTrigger;
	}
	else if (Button == RT)
	{
		return XINPUT_GAMEPAD_TRIGGER_THRESHOLD < xinputState[ControllerIdx].Gamepad.bRightTrigger;
	}
	else
	{
		return xinputState[ControllerIdx].Gamepad.wButtons & Button;
	}
	assert(0);
	return false;
}

bool UsersInput::ControllerInput(const int& ControllerIdx, XBOX_STICK StickInput, const float& DeadRange, const Vec2<float>& DeadRate)
{
	Vec2<float>vec;
	bool isLeftStick = StickInput <= L_RIGHT;
	if (isLeftStick)
	{
		vec = Vec2<float>(xinputState[ControllerIdx].Gamepad.sThumbLX, xinputState[ControllerIdx].Gamepad.sThumbLY);
	}
	else
	{
		vec = Vec2<float>(xinputState[ControllerIdx].Gamepad.sThumbRX, xinputState[ControllerIdx].Gamepad.sThumbRY);
	}

	if (StickInDeadZone(vec, DeadRate))return false;

	if (StickInput % 4 == L_UP)
	{
		return DeadRange < (vec.y / STICK_INPUT_MAX);
	}
	else if (StickInput % 4 == L_DOWN)
	{
		return  vec.y / STICK_INPUT_MAX < -DeadRange;
	}
	else if (StickInput % 4 == L_RIGHT)
	{
		return DeadRange < (vec.x / STICK_INPUT_MAX);
	}
	else if (StickInput % 4 == L_LEFT)
	{
		return  vec.x / STICK_INPUT_MAX < -DeadRange;
	}

	assert(0);
	return false;
}

bool UsersInput::ControllerOffTrigger(const int& ControllerIdx, XBOX_BUTTON Button)
{
	//トリガー
	if (Button == LT)
	{
		return XINPUT_GAMEPAD_TRIGGER_THRESHOLD < oldXinputState[ControllerIdx].Gamepad.bLeftTrigger
			&& !ControllerInput(ControllerIdx, Button);
	}
	else if (Button == RT)
	{
		return XINPUT_GAMEPAD_TRIGGER_THRESHOLD < oldXinputState[ControllerIdx].Gamepad.bRightTrigger
			&& !ControllerInput(ControllerIdx, Button);
	}
	//ボタン
	else
	{
		return (oldXinputState[ControllerIdx].Gamepad.wButtons & Button)
			&& !ControllerInput(ControllerIdx, Button);
	}
	assert(0);
	return false;
}

bool UsersInput::ControllerOffTrigger(const int& ControllerIdx, XBOX_STICK StickInput, const float& DeadRange, const Vec2<float>& DeadRate)
{
	Vec2<float>oldVec;
	Vec2<float>vec;
	bool isLeftStick = StickInput <= L_RIGHT;
	if (isLeftStick)
	{
		oldVec = Vec2<float>(oldXinputState[ControllerIdx].Gamepad.sThumbLX, oldXinputState[ControllerIdx].Gamepad.sThumbLY);
		vec = Vec2<float>(xinputState[ControllerIdx].Gamepad.sThumbLX, xinputState[ControllerIdx].Gamepad.sThumbLY);
	}
	else
	{
		oldVec = Vec2<float>(oldXinputState[ControllerIdx].Gamepad.sThumbRX, oldXinputState[ControllerIdx].Gamepad.sThumbRY);
		vec = Vec2<float>(xinputState[ControllerIdx].Gamepad.sThumbRX, xinputState[ControllerIdx].Gamepad.sThumbRY);
	}

	if (!StickInDeadZone(oldVec, DeadRate))return false;
	if (StickInDeadZone(vec, DeadRate))return false;

	bool result = false;
	if (StickInput % 4 == L_UP)
	{
		result = DeadRange < (oldVec.y / STICK_INPUT_MAX)
			&& !(DeadRange < (vec.y / STICK_INPUT_MAX));
	}
	else if (StickInput % 4 == L_DOWN)
	{
		result = oldVec.y / STICK_INPUT_MAX < -DeadRange
			&& !(vec.y / STICK_INPUT_MAX < -DeadRange);
	}
	else if (StickInput % 4 == L_RIGHT)
	{
		result = DeadRange < (oldVec.x / STICK_INPUT_MAX)
			&& !(DeadRange < (vec.x / STICK_INPUT_MAX));
	}
	else if (StickInput % 4 == L_LEFT)
	{
		result = oldVec.x / STICK_INPUT_MAX < -DeadRange
			&& !(vec.x / STICK_INPUT_MAX < -DeadRange);
	}
	else
	{
		assert(0);
	}
	if (result)
	{
	}
	return result;
}

Vec2<float> UsersInput::GetLeftStickVec(const int& ControllerIdx, const Vec2<float>& DeadRate)
{
	Vec2<float>result(xinputState[ControllerIdx].Gamepad.sThumbLX, -xinputState[ControllerIdx].Gamepad.sThumbLY);
	StickInDeadZone(result, DeadRate);
	return result.GetNormal();
}

Vec2<float> UsersInput::GetRightStickVec(const int& ControllerIdx, const Vec2<float>& DeadRate)
{
	Vec2<float>result(xinputState[ControllerIdx].Gamepad.sThumbRX, -xinputState[ControllerIdx].Gamepad.sThumbRY);
	StickInDeadZone(result, DeadRate);
	return result.GetNormal();
}

void UsersInput::ShakeController(const int& ControllerIdx, const float& Power, const int& Span)
{
	if (!(0 < Power && Power <= 1.0f))assert(0);
	shakePower[ControllerIdx] = Power;
	shakeTimer[ControllerIdx] = Span;
}
