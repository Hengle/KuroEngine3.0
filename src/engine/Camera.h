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
		Matrix matView; // ビュープロジェクション行列
		Matrix matProjection;
		Vec3<float> eye; // カメラ座標（ワールド座標）
		Matrix billboardMat;
		Matrix billboardMatY;
	};
	std::shared_ptr<ConstantBuffer>buff;

	Vec3<float>pos = { 0,0,-10 };
	Vec3<float>target = { 0,0,0 };
	Vec3<float>up = { 0,1,0 };
	Angle angleOfView = Angle(60);	//画角

	//マッピングの判断用ダーティフラグ
	bool dirty = true;

public:
	const std::string name;
	Camera(const std::string& Name);

	const std::shared_ptr<ConstantBuffer>&GetBuff();
};

