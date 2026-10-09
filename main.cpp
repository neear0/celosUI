#include <iostream>

#include "ui/win32/ui_win32.h"

int main(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmdline, int cmdshow) {
    HWND ui = celosia_win32::window::create(celosia_win32::variables::main_window_title, ImVec2(0, 0), celosia::ui::size, WS_EX_TOPMOST | WS_EX_LAYERED);

    celosia_win32::d3d::create_device(ui);
    celosia::initialize::context(ui);
    celosia::initialize::fonts();

    celosia_win32::window::enable_transparency(ui);
    celosia_win32::window::enable_blur(ui, 4);
    celosia_win32::window::show(ui);

    bool running = false;
    while (!running)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = true;
        }

        if (running)
            break;

        celosia::ui::render();
        if(celosia::ui::active)
            celosia_win32::window::drag();

        BringWindowToTop(ui);
    }

    // unload everything
    celosia_win32::window::destroy();
	return 0;
}
