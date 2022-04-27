#include "DrawFunc3D.h"
#include"KuroEngine.h"
#include"Model.h"
#include"LightManager.h"

//DrawLine
int DrawFunc3D::DRAW_LINE_COUNT = 0;
//DrawNonShadingModel
int DrawFunc3D::DRAW_NON_SHADING_COUNT = 0;
//DrawShadingModel
int DrawFunc3D::DRAW_SHADING_COUNT = 0;

void DrawFunc3D::DrawLine(Camera& Cam, const Vec3<float>& From, const Vec3<float>& To, const Color& LineColor, const float& Thickness, const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<VertexBuffer>>LINE_VERTEX_BUFF;

	//DrawLine��p���_
	class LineVertex
	{
	public:
		Vec3<float>fromPos;
		Vec3<float>toPos;
		Color color;
		float thickness;
		LineVertex(const Vec3<float>& FromPos, const Vec3<float>& ToPos, const Color& Color, const float& Thickness)
			:fromPos(FromPos), toPos(ToPos), color(Color), thickness(Thickness) {}
	};

	//�p�C�v���C��������
	if (!PIPELINE[BlendMode])
	{
		//�p�C�v���C���ݒ�
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.calling = false;

		//�V�F�[�_�[���
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/engine/DrawLine3D.hlsl", "VSmain", "vs_5_0");
		SHADERS.gs = D3D12App::Instance()->CompileShader("resource/engine/DrawLine3D.hlsl", "GSmain", "gs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/engine/DrawLine3D.hlsl", "PSmain", "ps_5_0");

		//�C���v�b�g���C�A�E�g
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("FROM_POS",DXGI_FORMAT_R32G32B32_FLOAT),
			InputLayoutParam("TO_POS",DXGI_FORMAT_R32G32B32_FLOAT),
			InputLayoutParam("COLOR",DXGI_FORMAT_R32G32B32A32_FLOAT),
			InputLayoutParam("THICKNESS",DXGI_FORMAT_R32_FLOAT),
		};

		//���[�g�p�����[�^
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�J�������o�b�t�@"),
		};

		//�����_�[�^�[�Q�b�g�`�����
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };
		//�p�C�v���C������
		PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(false, false));
	}

	KuroEngine::Instance().Graphics().SetPipeline(PIPELINE[BlendMode]);

	if (LINE_VERTEX_BUFF.size() < (DRAW_LINE_COUNT + 1))
	{
		LINE_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(LineVertex), 1, nullptr, ("DrawLine3D -" + std::to_string(DRAW_LINE_COUNT)).c_str()));
	}

	LineVertex vertex(From, To, LineColor, Thickness);
	LINE_VERTEX_BUFF[DRAW_LINE_COUNT]->Mapping(&vertex);
	Vec3<float>center = From.GetCenter(To);

	KuroEngine::Instance().Graphics().ObjectRender(
		LINE_VERTEX_BUFF[DRAW_LINE_COUNT],
		{
			Cam.GetBuff(),
		},
		{ CBV },
		center.z,
		true);

	DRAW_LINE_COUNT++;
}

void DrawFunc3D::DrawNonShadingModel(const std::weak_ptr<Model> Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<ConstantBuffer>>TRANSFORM_BUFF;

	//�p�C�v���C��������
	if (!PIPELINE[BlendMode])
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
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };
		//�p�C�v���C������
		PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, ModelMesh::Vertex_Model::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(false, false));
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
	if (!PIPELINE[BlendMode])
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
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };
		//�p�C�v���C������
		PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, ModelMesh::Vertex_Model::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(false, false));
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