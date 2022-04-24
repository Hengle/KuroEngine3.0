#pragma once
#include"Camera.h"
class DebugCamera
{
	Camera cam;

	Vec2<float>rotate = { 0.0f,0.0f };
	//カメラ～注視点までの距離
	float dist = 20.0f;
	//スケーリング
	Vec2<float>scale;
	//回転行列
	Matrix matRot = XMMatrixIdentity();

	void MoveXMVector(const XMVECTOR& MoveVector);

public:
	DebugCamera();
	void Init(const Vec3<float>& InitPos, const Vec3<float>& Target);
	void Move();

	operator Camera& ()
	{
		return cam;
	}
};