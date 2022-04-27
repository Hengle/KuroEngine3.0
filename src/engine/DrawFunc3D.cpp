#include "DrawFunc3D.h"
#include"KuroEngine.h"
#include"Model.h"
#include"LightManager.h"

//DrawNonShadingModel
int DrawFunc3D::DRAW_NON_SHADING_COUNT = 0;
//DrawShadingModel
int DrawFunc3D::DRAW_SHADING_COUNT = 0;

void DrawFunc3D::DrawNonShadingModel(const std::weak_ptr<Model> Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<ConstantBuffer>>TRANSFORM_BUFF;

	//�p�C�v���C��������
	if (!PIPELINE[0])
	{
		//�p�C�v���C���ݒ�
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�V�F�[�_�[���
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/engine/DrawNonShadingModel.hlsl", "VSmain", "vs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/engine/DrawNonShadingModel.hlsl", "PSmain", "ps_5_0");

		//���[�g�p�����[�^
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�J�������o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�g�����X�t�H�[���o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"�J���[�e�N�X�`��"),
		};

		//�����_�[�^�[�Q�b�g�`�����
		for (int i = 0; i < AlphaBlendModeNum; ++i)
		{
			std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), (AlphaBlendMode)i) };
			//�p�C�v���C������
			PIPELINE[i] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, ModelMesh::Vertex_Model::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(false, false));
		}
	}

	KuroEngine::Instance().Graphics().SetPipeline(PIPELINE[BlendMode]);

	if (TRANSFORM_BUFF.size() < (DRAW_NON_SHADING_COUNT + 1))
	{
		TRANSFORM_BUFF.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("DrawShadingModel_Transform -" + std::to_string(DRAW_NON_SHADING_COUNT)).c_str()));
	}

	TRANSFORM_BUFF[DRAW_NON_SHADING_COUNT]->Mapping(&Transform.GetMat());

	auto model = Model.lock();

	for (int meshIdx = 0; meshIdx < model->meshes.size(); ++meshIdx)
	{
		const auto& mesh = model->meshes[meshIdx];
		KuroEngine::Instance().Graphics().ObjectRender(
			mesh.mesh->vertBuff,
			mesh.mesh->idxBuff,
			{
				Cam.GetBuff(),
				TRANSFORM_BUFF[DRAW_NON_SHADING_COUNT],
				mesh.material->texBuff[COLOR_TEX],
			},
			{ CBV,CBV,SRV },
			Transform.GetPos().z,
			true);
	}

	DRAW_NON_SHADING_COUNT++;
}

void DrawFunc3D::DrawShadingModel(LightManager& LigManager, const std::weak_ptr<Model> Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<ConstantBuffer>>TRANSFORM_BUFF;

	//�p�C�v���C��������
	if (!PIPELINE[0])
	{
		//�p�C�v���C���ݒ�
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�V�F�[�_�[���
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/engine/DrawShadingModel.hlsl", "VSmain", "vs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/engine/DrawShadingModel.hlsl", "PSmain", "ps_5_0");

		//���[�g�p�����[�^
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�J�������o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�A�N�e�B�u���̃��C�g���o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�f�B���N�V�������C�g��� (�\�����o�b�t�@)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�|�C���g���C�g��� (�\�����o�b�t�@)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�X�|�b�g���C�g��� (�\�����o�b�t�@)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�V�����C�g��� (�\�����o�b�t�@)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�g�����X�t�H�[���o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"�J���[�e�N�X�`��"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�}�e���A����{���o�b�t�@"),

		};

		//�����_�[�^�[�Q�b�g�`�����
		for (int i = 0; i < AlphaBlendModeNum; ++i)
		{
			std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), (AlphaBlendMode)i) };
			//�p�C�v���C������
			PIPELINE[i] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, ModelMesh::Vertex_Model::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(false, false));
		}
	}

	KuroEngine::Instance().Graphics().SetPipeline(PIPELINE[BlendMode]);

	if (TRANSFORM_BUFF.size() < (DRAW_SHADING_COUNT + 1))
	{
		TRANSFORM_BUFF.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("DrawShadingModel_Transform -" + std::to_string(DRAW_SHADING_COUNT)).c_str()));
	}

	TRANSFORM_BUFF[DRAW_SHADING_COUNT]->Mapping(&Transform.GetMat());

	auto model = Model.lock();

	for (int meshIdx = 0; meshIdx < model->meshes.size(); ++meshIdx)
	{
		const auto& mesh = model->meshes[meshIdx];
		KuroEngine::Instance().Graphics().ObjectRender(
			mesh.mesh->vertBuff,
			mesh.mesh->idxBuff,
			{
				Cam.GetBuff(),
				LigManager.GetLigNumInfo(),
				LigManager.GetLigInfo(Light::DIRECTION),
				LigManager.GetLigInfo(Light::POINT),
				LigManager.GetLigInfo(Light::SPOT),
				LigManager.GetLigInfo(Light::HEMISPHERE),
				TRANSFORM_BUFF[DRAW_NON_SHADING_COUNT],
				mesh.material->texBuff[COLOR_TEX],
				mesh.material->buff,
			},
			{ CBV,CBV,SRV,SRV,SRV,SRV,CBV,SRV,CBV },
			Transform.GetPos().z,
			true);
	}

	DRAW_NON_SHADING_COUNT++;
}
