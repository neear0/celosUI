#include "../../ui.h"

// the box's border in the text color, faint, so an unchecked box shows over the os blur and on both themes. checked, the border
// and the mark ease into the main color, unchecked the mark fades out
static void render_check_mark_animated(ImGuiID id, bool active, bool hovered, bool held, ImDrawList* drawlist, ImRect check_bb, float pad, float square_sz) {
    const ImVec4 text = celosia::style::themes::active.text.Value;
    ImVec4 border = ImVec4(text.x, text.y, text.z, held ? 0.3f : hovered ? 0.45f : 0.25f);
    ImVec4 mark = celosia::style::general::main_color.Value;
    if (active)
        border = mark;
    else
        mark.w = 0.f;

    const ImColor border_animated = celosia::animations::set(celosia::animations::key("checkbox_border", id), border, celosia::animations::color_speed);
    const ImColor mark_animated = celosia::animations::set(celosia::animations::key("checkbox", id), mark, celosia::animations::color_speed);

    ImGui::RenderCheckMark(drawlist, check_bb.Min + ImVec2(pad, pad), mark_animated, square_sz - pad * 2.0f);
    celosia::widgets::render_frame_border_color(check_bb.Min, check_bb.Max, border_animated, (float)celosia::style::general::rounding);
}

bool ImGui::Checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = IM_TRUNC(g.FontSize * 1.15f); // follows the text size, not the frame padding
    const float row_height = ImMax(square_sz, label_size.y); // the box and the label are centered on the row
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), row_height + style.FramePadding.y));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    const ImVec2 check_pos(pos.x, pos.y + IM_TRUNC((row_height - square_sz) * 0.5f));
    const ImRect check_bb(check_pos, check_pos + ImVec2(square_sz, square_sz));
    RenderNavHighlight(total_bb, id);

    const float pad = ImMax(1.0f, IM_TRUNC(square_sz / 6.0f));
    render_check_mark_animated(id, *v, hovered, held, window->DrawList, check_bb, pad, square_sz);

    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, pos.y + IM_TRUNC((row_height - label_size.y) * 0.5f));
    if (g.LogEnabled)
        LogRenderedText(&label_pos, *v ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)
        RenderText(label_pos, label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool celosia::widgets::checkbox_map(const char* label) { return ImGui::Checkbox(label, &celosia::variables::config::bools[label]); }
