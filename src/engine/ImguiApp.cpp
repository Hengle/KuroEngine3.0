#include "ImguiApp.h"

ImguiApp* ImguiApp::INSTANCE = nullptr;

ImguiApp::ImguiApp(const ComPtr<ID3D12Device>& Device, const HWND& Hwnd)
{
	if (INSTANCE != nullptr)
	{
		printf("既にインスタンスがあります。ImguiAppは１つのインスタンスしか持てません\n");
		assert(0);
	}
	INSTANCE = this;

	// SRV_CBV_UAV のディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	HRESULT result = Device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(heap.GetAddressOf()));

	if (heap == nullptr)assert(0);

	if (ImGui::CreateContext() == nullptr)assert(0);

	bool blnResult = ImGui_ImplWin32_Init(Hwnd);
	if (!blnResult)assert(0);

	blnResult = ImGui_ImplDX12_Init(
		Device.Get(),
		2,
		DXGI_FORMAT_R8G8B8A8_UNORM,	//書き込み先のRTVのフォーマット
		heap.Get(),
		heap.Get()->GetCPUDescriptorHandleForHeapStart(),	//CPUハンドル
		heap.Get()->GetGPUDescriptorHandleForHeapStart());	//GPUハンドル
}

ImguiApp::~ImguiApp()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImguiApp::BeginImgui()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImguiApp::EndImgui(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	ImGui::Render();
	CmdList->SetDescriptorHeaps(1, heap.GetAddressOf());

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CmdList.Get());
}