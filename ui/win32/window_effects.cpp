#include "../win32/ui_win32.h"

namespace celosia_win32::window {
    void enable_transparency(HWND hwnd) { // 0, 0, 0, 0 will turn invisible
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
        RECT clarea{}; GetClientRect(hwnd, &clarea);
        RECT warea{}; GetWindowRect(hwnd, &warea);

        POINT difference{};
        ClientToScreen(hwnd, &difference);

        const MARGINS margin{
            warea.left + difference.x - warea.left,
            warea.top + difference.y - warea.top,
            clarea.right, clarea.bottom
        };

        DwmExtendFrameIntoClientArea(hwnd, &margin);
    }

    void enable_blur(HWND hwnd, int accent) {
        struct accent_policy {
            int accent_state;
            int accent_flags;
            int gradient_color;
            int animation_id;
        };
        struct win_comp_attr_data {
            int attribute;
            PVOID data;
            ULONG size;
        };

        const HINSTANCE hm = LoadLibrary(L"user32.dll");
        if (hm) {
            typedef BOOL(WINAPI* set_window_composition_attribute_fn)(HWND, win_comp_attr_data*);

            const set_window_composition_attribute_fn set_window_composition_attribute = (set_window_composition_attribute_fn)GetProcAddress(hm, "SetWindowCompositionAttribute");
            if (set_window_composition_attribute) {
                accent_policy policy = { accent, 0, 0, 0 };
                win_comp_attr_data data = { 19, &policy,sizeof(accent_policy) };
                set_window_composition_attribute(hwnd, &data);
            }
            FreeLibrary(hm);
        }
    }

}