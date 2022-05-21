#pragma once
#include"Vec.h"
#include"KuroMath.h"
#include<memory>
class ConstantBuffer;
#include<string>

class Camera
{
	class ConstData
	{
	public:
		Matrix matView; // �r���[�s��
		Matrix matProjection;	//�v���W�F�N�V�����s��
		Vec3<float> eye; // �J�������W�i���[���h���W�j
		Matrix billboardMat;
		Matrix billboardMatY;
	}cameraInfo;
	std::shared_ptr<ConstantBuffer>buff;

	Vec3<float>pos = { 0,0,-10 };
	Vec3<float>target = { 0,0,0 };
	Vec3<float>up = { 0,1,0 };
	Angle angleOfView = Angle(60);	//��p
	float nearZ = 0.1f;
	float farZ = 3000.0f;

	Matrix viewInvMat; //�r���[�s��̋t�s��

	//�}�b�s���O�̔��f�p�_�[�e�B�t���O
	bool dirty = true;
	void CameraInfoUpdate();

public:
	const std::string name;
	Camera(const std::string& Name);

	//�Z�b�^
	void SetPos(const Vec3<float>& Pos) 
	{
		pos = Pos;
		dirty = true;
	}
	void SetUp(const Vec3<float>& Up)
	{
		up = Up;
		dirty = true;
	}
	void SetTarget(const Vec3<float>& Target) 
	{
		target = Target;
		dirty = true;
	}
	void SetAngleOfView(const Angle& Angle)
	{
		angleOfView = Angle;
		dirty = true;
	}

	//�Q�b�^
	const Vec3<float>& GetPos() { return pos; }
	const Vec3<float>& GetUp() { return up; }
	const Vec3<float>& GetTarget() { return target; }
	const Angle& GetAngleOfView() { return angleOfView; }
	const Matrix& GetViewMat() { CameraInfoUpdate(); return cameraInfo.matView; }
	const Matrix& GetProjectionMat() { CameraInfoUpdate(); return cameraInfo.matProjection; }
	const Vec3<float>GetForward();
	const Vec3<float>GetRight();
	const float& GetNearZ() { return nearZ; }
	const float& GetFarZ() { return farZ; }

	const std::shared_ptr<ConstantBuffer>&GetBuff();
};

