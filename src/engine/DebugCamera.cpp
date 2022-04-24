#include "DebugCamera.h"
#include"WinApp.h"
#include"UsersInput.h"

void DebugCamera::MoveXMVector(const XMVECTOR& MoveVector)
{
	auto pos = cam.GetPos();
	auto target = cam.GetTarget();

	Vec3<float>move(MoveVector.m128_f32[0], MoveVector.m128_f32[1], MoveVector.m128_f32[2]);
	pos += move;
	target += move;

	cam.SetPos(pos);
	cam.SetTarget(target);
}

DebugCamera::DebugCamera()
	:cam("DebugCamera")
{
	dist = cam.GetPos().Distance(cam.GetTarget());

	//��ʃT�C�Y�ɑ΂��鑊�ΓI�ȃX�P�[���ɒ���
	scale.x = 1.0f / (float)WinApp::Instance()->GetExpandWinSize().x;
	scale.y = 1.0f / (float)WinApp::Instance()->GetExpandWinSize().y;
}

void DebugCamera::Init(const Vec3<float>& InitPos, const Vec3<float>& Target)
{
	cam.SetPos(InitPos);
	cam.SetTarget(Target);

	dist = InitPos.Distance(Target);
}

void DebugCamera::Move()
{
	bool moveDirty = false;
	float angleX = 0;
	float angleY = 0;

	//�}�E�X�̓��͂��擾
	UsersInput::MouseMove mouseMove = UsersInput::Instance()->GetMouseMove();

	//�}�E�X���N���b�N�ŃJ������]
	if (UsersInput::Instance()->MouseInput(MOUSE_BUTTON::RIGHT))
	{
		float dy = mouseMove.IX * scale.y;
		float dx = mouseMove.IY * scale.x;

		angleX = -dx * XM_PI;
		angleY = -dy * XM_PI;
		rotate.x += XMConvertToDegrees(-angleX);
		rotate.y += XMConvertToDegrees(-angleY);
		moveDirty = true;
	}

	//�}�E�X���N���b�N�ŃJ�������s�ړ�
	if (UsersInput::Instance()->MouseInput(MOUSE_BUTTON::CENTER))
	{
		float dx = mouseMove.IX / 100.0f;
		float dy = mouseMove.IY / 100.0f;

		XMVECTOR move = { -dx,+dy,0,0 };
		move = XMVector3Transform(move, matRot);

		MoveXMVector(move);
		moveDirty = true;
	}

	//�z�C�[�����͂ŋ�����ύX
	if (mouseMove.IZ != 0)
	{
		dist -= mouseMove.IZ / 100.0f;
		dist = max(dist, 1.0f);
		moveDirty = true;
	}

	if (moveDirty)
	{
		//�ǉ���]���̉�]�s��𐶐�
		XMMATRIX matRotNew = XMMatrixIdentity();
		matRotNew *= XMMatrixRotationX(-angleX);
		matRotNew *= XMMatrixRotationY(-angleY);
		// ����]�s���ݐς��Ă����ƁA�덷�ŃX�P�[�����O��������댯�������
		// �N�H�[�^�j�I�����g�p��������]�܂���
		matRot = matRotNew * matRot;

		// �����_���王�_�ւ̃x�N�g���ƁA������x�N�g��
		XMVECTOR vTargetEye = { 0.0f, 0.0f, -dist, 1.0f };
		XMVECTOR vUp = { 0.0f, 1.0f, 0.0f, 0.0f };

		// �x�N�g������]
		vTargetEye = XMVector3Transform(vTargetEye, matRot);
		vUp = XMVector3Transform(vUp, matRot);

		// �����_���炸�炵���ʒu�Ɏ��_���W������
		Vec3<float>target = cam.GetTarget();
		cam.SetPos({ target.x + vTargetEye.m128_f32[0], target.y + vTargetEye.m128_f32[1], target.z + vTargetEye.m128_f32[2] });
		cam.SetUp({ vUp.m128_f32[0], vUp.m128_f32[1], vUp.m128_f32[2] });
	}
}
