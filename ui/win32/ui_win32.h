#pragma once

#include "../ui.h"
#include <WinUser.h>

namespace celosia_win32 {
	namespace d3d {
		bool create_device(HWND hwnd);
	}

	namespace variables {
		inline std::map<LPCWSTR, std::pair<WNDCLASSEXW, HWND>> windows;
		inline long default_window_flags = WS_EX_TOPMOST | WS_EX_LAYERED; // WS_EX_TRANSPARENT to be able to click through
		inline LPCWSTR main_window_title = L"Celosia UI";
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
		void drag();
	}
}