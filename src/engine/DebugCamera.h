#pragma once
#include"Camera.h"
class DebugCamera
{
	Camera cam;

	Vec2<float>rotate = { 0.0f,0.0f };
	//�J�����`�����_�܂ł̋���
	float dist = 20.0f;
	//�X�P�[�����O
	Vec2<float>scale;
	//��]�s��
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