#include "DrawFunc2D_FillTex.h"
#include"KuroEngine.h"

//DrawExtendGraph
std::shared_ptr<GraphicsPipeline>DrawFunc2D_FillTex::EXTEND_GRAPH_PIPELINE;
int DrawFunc2D_FillTex::DRAW_EXTEND_GRAPH_COUNT = 0;
std::vector<std::shared_ptr<VertexBuffer>>DrawFunc2D_FillTex::EXTEND_GRAPH_VERTEX_BUFF;

//DrawRotaGraph
std::shared_ptr<GraphicsPipeline>DrawFunc2D_FillTex::ROTA_GRAPH_PIPELINE;
int DrawFunc2D_FillTex::DRAW_ROTA_GRAPH_COUNT = 0;
std::vector<std::shared_ptr<VertexBuffer>>DrawFunc2D_FillTex::ROTA_GRAPH_VERTEX_BUFF;

static std::vector<RootParam>ROOT_PARAMETER =
{
	RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "平行投影行列定数バッファ"),
	RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "元のテクスチャリソース"),
	RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "塗りつぶす用テクスチャリソース"),
};

void DrawFunc2D_FillTex::CreateExtendGraphFunc()
{
	//パイプライン未生成
	if (!EXTEND_GRAPH_PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph_FillTex.hlsl", "VSmain", "vs_5_0");
		SHADERS.gs = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph_FillTex.hlsl", "GSmain", "gs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph_FillTex.hlsl", "PSmain", "ps_5_0");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION_L_U",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("POSITION_R_B",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("SRC_ALPHA",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT),
			InputLayoutParam("PAINT_UV_L_U",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("PAINT_UV_R_B",DXGI_FORMAT_R32G32_FLOAT)
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans) };
		//パイプライン生成
		EXTEND_GRAPH_PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(true, false));
	}
}

void DrawFunc2D_FillTex::DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& DestTex, const std::shared_ptr<TextureBuffer>& SrcTex, 
	const float& SrcAlpha, const Vec2<bool>& Miror, const Vec2<float>& LeftUpPaintUV, const Vec2<float>& RightBottomPaintUV)
{
	CreateExtendGraphFunc();

	KuroEngine::Instance().Graphics().SetPipeline(EXTEND_GRAPH_PIPELINE);

	if (EXTEND_GRAPH_VERTEX_BUFF.size() < (DRAW_EXTEND_GRAPH_COUNT + 1))
	{
		EXTEND_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(ExtendGraphVertex), 1, nullptr, ("DrawExtendGraph_FillTex -" + std::to_string(DRAW_EXTEND_GRAPH_COUNT)).c_str()));
	}

	auto rightBottomPos = LeftUpPos;
	rightBottomPos.x += DestTex->GetDesc().Width;
	rightBottomPos.y += DestTex->GetDesc().Height;
	ExtendGraphVertex vertex(LeftUpPos, rightBottomPos, SrcAlpha, Miror, LeftUpPaintUV, RightBottomPaintUV);
	EXTEND_GRAPH_VERTEX_BUFF[DRAW_EXTEND_GRAPH_COUNT]->Mapping(&vertex);

	KuroEngine::Instance().Graphics().ObjectRender(EXTEND_GRAPH_VERTEX_BUFF[DRAW_EXTEND_GRAPH_COUNT], 
		{ KuroEngine::Instance().GetParallelMatProjBuff(),DestTex,SrcTex }, { CBV,SRV,SRV }, 0.0f, true);

	DRAW_EXTEND_GRAPH_COUNT++;
}

void DrawFunc2D_FillTex::DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const std::shared_ptr<TextureBuffer>& DestTex, const std::shared_ptr<TextureBuffer>& SrcTex,
	const float& SrcAlpha, const Vec2<bool>& Miror, const Vec2<float>& LeftUpPaintUV, const Vec2<float>& RightBottomPaintUV)
{
	if (RightBottomPaintUV.x < LeftUpPaintUV.x)assert(0);
	if (RightBottomPaintUV.y < LeftUpPaintUV.y)assert(0);

	CreateExtendGraphFunc();

	KuroEngine::Instance().Graphics().SetPipeline(EXTEND_GRAPH_PIPELINE);

	if (EXTEND_GRAPH_VERTEX_BUFF.size() < (DRAW_EXTEND_GRAPH_COUNT + 1))
	{
		EXTEND_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(ExtendGraphVertex), 1, nullptr, ("DrawExtendGraph_FillTex -" + std::to_string(DRAW_EXTEND_GRAPH_COUNT)).c_str()));
	}

	ExtendGraphVertex vertex(LeftUpPos, RightBottomPos, SrcAlpha, Miror, LeftUpPaintUV, RightBottomPaintUV);
	EXTEND_GRAPH_VERTEX_BUFF[DRAW_EXTEND_GRAPH_COUNT]->Mapping(&vertex);

	KuroEngine::Instance().Graphics().ObjectRender(EXTEND_GRAPH_VERTEX_BUFF[DRAW_EXTEND_GRAPH_COUNT], 
		{ KuroEngine::Instance().GetParallelMatProjBuff(),DestTex,SrcTex }, { CBV,SRV,SRV }, 0.0f, true);

	DRAW_EXTEND_GRAPH_COUNT++;
}

void DrawFunc2D_FillTex::DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian, 
	const std::shared_ptr<TextureBuffer>& DestTex, const std::shared_ptr<TextureBuffer>& SrcTex, const float& SrcAlpha,
	const Vec2<float>& RotaCenterUV, const Vec2<bool>& Miror,	const Vec2<float>& LeftUpPaintUV, const Vec2<float>& RightBottomPaintUV)
{
	if (RightBottomPaintUV.x < LeftUpPaintUV.x)assert(0);
	if (RightBottomPaintUV.y < LeftUpPaintUV.y)assert(0);

	//DrawRotaGraph専用頂点
	class RotaGraphVertex
	{
	public:
		Vec2<float>center;
		Vec2<float> extRate;
		float radian;
		Vec2<float>rotaCenterUV;
		float alpha;
		Vec2<int> miror;
		Vec2<float>leftUpPaintUV;
		Vec2<float>rightBottomPaintUV;
		RotaGraphVertex(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
			const Vec2<float>& RotaCenterUV, const float& Alpha, const Vec2<bool>& Miror,
			const Vec2<float>& LeftUpPaintUV, const Vec2<float>& RightBottomPaintUV)
			:center(Center), extRate(ExtRate), radian(Radian), rotaCenterUV(RotaCenterUV), alpha(Alpha), miror({ Miror.x ? 1 : 0,Miror.y ? 1 : 0 }),
			leftUpPaintUV(LeftUpPaintUV), rightBottomPaintUV(RightBottomPaintUV) {}
	};

	//パイプライン未生成
	if (!ROTA_GRAPH_PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph_FillTex.hlsl", "VSmain", "vs_5_0");
		SHADERS.gs = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph_FillTex.hlsl", "GSmain", "gs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph_FillTex.hlsl", "PSmain", "ps_5_0");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("EXT_RATE",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("RADIAN",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("ROTA_CENTER_UV",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("SRC_ALPHA",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT),
			InputLayoutParam("PAINT_UV_L_U",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("PAINT_UV_R_B",DXGI_FORMAT_R32G32_FLOAT)
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans) };
		//パイプライン生成
		ROTA_GRAPH_PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(true, false));
	}

	KuroEngine::Instance().Graphics().SetPipeline(ROTA_GRAPH_PIPELINE);

	if (ROTA_GRAPH_VERTEX_BUFF.size() < (DRAW_ROTA_GRAPH_COUNT + 1))
	{
		ROTA_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(RotaGraphVertex), 1, nullptr, ("DrawRotaGraph_FillTex -" + std::to_string(DRAW_ROTA_GRAPH_COUNT)).c_str()));
	}

	RotaGraphVertex vertex(Center, ExtRate, Radian, RotaCenterUV, SrcAlpha, Miror, LeftUpPaintUV, RightBottomPaintUV);
	ROTA_GRAPH_VERTEX_BUFF[DRAW_ROTA_GRAPH_COUNT]->Mapping(&vertex);

	KuroEngine::Instance().Graphics().ObjectRender(ROTA_GRAPH_VERTEX_BUFF[DRAW_ROTA_GRAPH_COUNT],
		{ KuroEngine::Instance().GetParallelMatProjBuff(),DestTex,SrcTex }, { CBV,SRV,SRV }, 0.0f, true);

	DRAW_ROTA_GRAPH_COUNT++;
}
