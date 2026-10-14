#include "../../ui.h"

static void tab_style(ImGuiID id, const ImRect bb, const char* label, ImVec2 label_size, const ImGuiStyle& style, ImColor col) {
    const bool active = celosia::ui::tab_active == label;
    if (active)
        col = celosia::style::general::main_color;

    const float glow = celosia::animations::set(celosia::animations::key("tab_glow", id), active ? 1.f : 0.f);
    if (glow > 0.01f) { // the active tab glows in the main color, fading in and out
        const ImVec4 main = celosia::style::general::main_color.Value;
        celosia::effects::glow(ImGui::GetWindowDrawList(), bb.Min, bb.Max, ImColor(main.x, main.y, main.z, 0.5f * glow), style.FrameRounding, 10.f * celosia::style::scale);
    }

    celosia::widgets::render_frame_border_animated(id, bb.Min, bb.Max, col, style.FrameRounding);
    ImGui::RenderTextClipped(ImVec2(bb.Min.x + style.FramePadding.x, bb.Min.y + style.FramePadding.y), ImVec2(bb.Max.x - style.FramePadding.x, bb.Max.y - style.FramePadding.y), label, NULL, &label_size, style.ButtonTextAlign, &bb);
}

bool celosia::widgets::tab_button(const char* label, const ImVec2& size_arg)
{
    return tab_button_ex(label, size_arg, ImGuiButtonFlags_None);
}

bool celosia::widgets::tab_button_ex(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    const ImVec2 size( // 0 is the sidebar's width and the text's height
        size_arg.x > 0.f ? size_arg.x : celosia::style::sidebar::width - style.FramePadding.x * 2,
        size_arg.y > 0.f ? size_arg.y : label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    if(pressed) 
        celosia::ui::tab_active = label; // change tab

    const ImColor col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImGui::RenderNavHighlight(bb, id);
    tab_style(id, bb, label, label_size, style, col);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}
