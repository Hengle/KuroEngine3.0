#include "Transform2D.h"

std::vector<Transform2D*> Transform2D::TRANSFORMS;

Matrix* Transform2D::GetMat()
{
	bool parentDirty = (parent != nullptr && parent->dirty);
	if (mat != nullptr)
	{
		if (!parentDirty)return mat;
		else SAFE_DELETE(mat);
	}

	mat = new Matrix();
	*mat = XMMatrixScaling(scale.x, scale.y, 1.0f) * rotate;
	mat->r[3].m128_f32[0] = pos.x;
	mat->r[3].m128_f32[1] = pos.y;

	if (parent != nullptr)
	{
		*mat *= *parent->GetMat();
	}

	return mat;
}
