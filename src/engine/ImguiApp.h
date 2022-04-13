#pragma once
#include<wrl.h>
#include<d3d12.h>
#include"imgui.h"
#include"imgui_impl_win32.h"
#include"imgui_impl_dx12.h"
#include<stdio.h>

class ImguiApp
{
private:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static ImguiApp* INSTANCE;

public:
	static ImguiApp* Instance()
	{
		if (INSTANCE == nullptr)
		{
			printf("ImguiAppのインスタンスを呼び出そうとしましたがnullptrでした\n");
			assert(0);
		}
		return INSTANCE;
	}

private:
	ComPtr<ID3D12DescriptorHeap>heap;

public:
	ImguiApp(const ComPtr<ID3D12Device>& Device, const HWND& Hwnd);
	~ImguiApp();
	void BeginImgui();
	void EndImgui(const ComPtr<ID3D12GraphicsCommandList>& CmdList);
};

