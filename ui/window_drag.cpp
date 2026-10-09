#include "ui.h"

namespace celosia::window {
    static bool dragging = false;
    static ImVec2 offset;

    void drag() { // ctodo: make sure these aren't being handled when there's no ui visible, preferably in loop
        const ImRect window_rect = platform::window_rect();
        const ImVec2 mouse_pos = functions::mouse_vec2();

        if (dragging) {
            dragging = inputsystem::key::held(keys::mouse_left);
            platform::move(ImVec2(mouse_pos.x - offset.x, mouse_pos.y - offset.y));
            return;
        }

        if ((mouse_pos.x >= window_rect.Min.x && mouse_pos.x <= window_rect.Max.x) &&
            (mouse_pos.y >= window_rect.Min.y && mouse_pos.y <= window_rect.Min.y + style::titlebar::height)) {
            if (inputsystem::key::down(keys::mouse_left) && !dragging) {
                dragging = true;
                offset = { mouse_pos.x - window_rect.Min.x, mouse_pos.y - window_rect.Min.y };
            }
        }

        if (inputsystem::key::up(keys::mouse_left) && dragging)
            dragging = false;
    }
}
