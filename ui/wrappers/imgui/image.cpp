#include "../../ui.h"

void celosia::widgets::image(const char* name, ImVec2 size, float rounding) {
    const celosia::resources::images::t_image image = celosia::resources::images::get(name); // a copy, it's a pointer and a size
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!image.texture || window->SkipItems)
        return;
    if (size.x <= 0.f || size.y <= 0.f)
        size = image.size;

    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, 0))
        return;
    window->DrawList->AddImageRounded(image.texture, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, rounding);
}
