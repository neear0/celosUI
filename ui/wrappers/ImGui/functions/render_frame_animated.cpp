#include "../../../ui.h"

void ImGui::render_frame_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, bool border, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const ImGuiID id = celosia::animations::key("render_frame_animated", celosia::animations::key(label));
    ImColor animated_color = celosia::animations::set(id, ImVec4(fill_col), celosia::animations::color_speed);
    window->DrawList->AddRectFilled(p_min, p_max, animated_color, rounding);
}