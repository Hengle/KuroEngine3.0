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
		Matrix matView; // ビュー行列
		Matrix matProjection;	//プロジェクション行列
		Vec3<float> eye; // カメラ座標（ワールド座標）
		Matrix billboardMat;
		Matrix billboardMatY;
	}cameraInfo;
	std::shared_ptr<ConstantBuffer>buff;

	Vec3<float>pos = { 0,0,-10 };
	Vec3<float>target = { 0,0,0 };
	Vec3<float>up = { 0,1,0 };
	Angle angleOfView = Angle(60);	//画角
	float nearZ = 0.1f;
	float farZ = 3000.0f;

	Matrix viewInvMat; //ビュー行列の逆行列

	//マッピングの判断用ダーティフラグ
	bool dirty = true;
	void CameraInfoUpdate();

public:
	const std::string name;
	Camera(const std::string& Name);

	//セッタ
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

	//ゲッタ
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

