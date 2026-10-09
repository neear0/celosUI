#include "../../../ui.h"
#include "../../../ui.h"

void ImGui::render_frame_border_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, float rounding) // any component using this will be animated with no need for any further setup
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;


    const ImGuiID id = celosia::animations::key("render_frame_border_animated", celosia::animations::key(label));
    ImColor animated_color = celosia::animations::set(id, ImVec4(fill_col), 0.f, celosia::animations::e_method::linear);

    int border_size = celosia::style::frame::border_size;
    //window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), animated_color, rounding, 0, border_size);
    window->DrawList->AddRect(p_min, p_max, animated_color, rounding, 0, border_size);
}