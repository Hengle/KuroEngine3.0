#include "DrawFunc3D.h"
#include"KuroEngine.h"
#include"Model.h"

//DrawModel
//std::shared_ptr<GraphicsPipeline>DrawFunc3D::DEFAULT_PIPELINE[AlphaBlendModeNum];
int DrawFunc3D::DRAW_NON_SHADING_COUNT = 0;
//std::vector<std::shared_ptr<ConstantBuffer>>DrawFunc3D::DEFAULT_TRANSFORM_BUFF;

void DrawFunc3D::DrawNonShadingModel(const std::weak_ptr<Model> Model, Transform& Transform, Camera& Cam, const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<ConstantBuffer>>TRANSFORM_BUFF;

	//パイプライン未生成
	if (!PIPELINE[0])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/engine/DrawNonShadingModel.hlsl", "VSmain", "vs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/engine/DrawNonShadingModel.hlsl", "PSmain", "ps_5_0");

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"トランスフォームバッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"カラーテクスチャ"),
		};

		//レンダーターゲット描画先情報
		for (int i = 0; i < AlphaBlendModeNum; ++i)
		{
			std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), (AlphaBlendMode)i) };
			//パイプライン生成
			PIPELINE[i] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, ModelMesh::Vertex_Model::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(false, false));
		}
	}

	KuroEngine::Instance().Graphics().SetPipeline(PIPELINE[BlendMode]);

	if (TRANSFORM_BUFF.size() < (DRAW_NON_SHADING_COUNT + 1))
	{
		TRANSFORM_BUFF.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("DrawModel_Transform -" + std::to_string(DRAW_NON_SHADING_COUNT)).c_str()));
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
