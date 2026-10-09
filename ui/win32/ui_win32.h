#pragma once

#include "../ui.h"
#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>

#include "../../external/imgui/dx11/imgui_impl_dx11.h"
#include "../../external/imgui/win32/imgui_impl_win32.h"

#pragma comment(lib, "d3d11.lib")

namespace celosia_win32 {
	namespace d3d {
		inline ID3D11Device* device;
		inline ID3D11DeviceContext* device_context;
		inline IDXGISwapChain* swapchain;
		inline ID3D11RenderTargetView* render_target_view;

		bool create_device(HWND hwnd);
	}

	namespace variables {
		inline std::map<LPCWSTR, std::pair<WNDCLASSEXW, HWND>> windows;
		inline long default_window_flags = WS_EX_TOPMOST | WS_EX_LAYERED; // WS_EX_TRANSPARENT to be able to click through
	}

	namespace window {
		LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);
		HWND create(LPCWSTR window_title, ImVec2 pos, ImVec2 size, long window_flags = variables::default_window_flags);
		void enable_transparency(HWND hwnd);
		void enable_blur(HWND hwnd, int accent = 3);
		void show(HWND hwnd);
		void move(HWND hwnd, ImVec2 pos);
		void resize(HWND hwnd, ImVec2 size);
		void destroy();
	}
}