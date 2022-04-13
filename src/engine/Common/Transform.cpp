#include "Transform.h"

std::vector<Transform*> Transform::TRANSFORMS;

Matrix* Transform::GetMat(Matrix* BillBoardMat)
{
	bool parentDirty = (parent != nullptr && parent->dirty);
	if (mat != nullptr)
	{
		if (!parentDirty)return mat;
		else SAFE_DELETE(mat);
	}

	//•Ï‰»‚ ‚èA–¢ŒvŽZ
	mat = new Matrix();
	*mat = XMMatrixScaling(scale.x, scale.y, scale.z) * rotate;
	if (BillBoardMat != nullptr)
	{
		*mat *= *BillBoardMat;
	}
	*mat *= XMMatrixTranslation(pos.x, pos.y, pos.z);

	if (parent != nullptr)
	{
		*mat *= *(parent->GetMat());
	}

	return mat;
}
