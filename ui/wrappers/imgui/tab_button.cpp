#include "../../ui.h"

static void tab_style(ImGuiID id, const ImRect bb, const char* label, ImVec2 label_size, ImColor col) {
    const float rounding = (float)celosia::style::general::rounding;
    const ImVec2 padding = celosia::style::frame::size_padding;
    const bool active = celosia::ui::tab_active == label;
    if (active)
        col = celosia::style::general::accent;

    const float glow = celosia::animations::set(celosia::animations::key("tab_glow", id), active ? 1.f : 0.f);
    if (glow > 0.01f) { // the active tab glows in the accent, fading in and out
        const ImVec4 main = celosia::style::general::accent.Value;
        celosia::effects::glow(ImGui::GetWindowDrawList(), bb.Min, bb.Max, ImColor(main.x, main.y, main.z, 0.5f * glow), rounding, 10.f * celosia::style::scale);
    }

    celosia::widgets::render_frame_border_animated(id, bb.Min, bb.Max, col, rounding);
    celosia::render::text::draw_aligned(ImGui::GetWindowDrawList(), bb.Min + padding, bb.Max - padding, ImVec2(0.5f, 0.5f), celosia::style::themes::active.text, label, celosia::render::text::label_end(label), &label_size, &bb);
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
    const ImVec2 padding = style::frame::size_padding;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = celosia::render::text::size(label, celosia::render::text::label_end(label));

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && padding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - padding.y;
    const ImVec2 size( // 0 is the sidebar's width and the text's height
        size_arg.x > 0.f ? size_arg.x : style::sidebar::width - padding.x * 2,
        size_arg.y > 0.f ? size_arg.y : label_size.y + padding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, padding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    if(pressed) 
        celosia::ui::tab_active = label; // change tab

    const style::themes::t_theme& theme = style::themes::active;
    const ImColor col = (held && hovered) ? theme.held : hovered ? theme.hovered : theme.idle;
    ImGui::RenderNavHighlight(bb, id);
    tab_style(id, bb, label, label_size, col);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}
