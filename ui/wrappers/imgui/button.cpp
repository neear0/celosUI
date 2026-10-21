#include "../../ui.h"

static void style1(ImGuiID id, const ImRect bb, const char* label, ImVec2 label_size, const ImGuiStyle& style, ImColor col) {
    ImVec4 fill = ImLerp(col.Value, celosia::style::themes::active.background_darker.Value, 0.25f); // a bit towards the panel color, darker than the border on dark themes and lighter on light ones
    fill.w = col.Value.w;
    celosia::widgets::render_frame_animated(id, bb.Min, bb.Max, fill, style.FrameRounding);
    ImVec4 border = ImLerp(col.Value, celosia::style::themes::active.text.Value, 0.12f); // a little towards the text color, so the button shows over a dark desktop through the os blur
    border.w = col.Value.w;
    celosia::widgets::render_frame_border_animated(id, bb.Min, bb.Max, border, style.FrameRounding);
    celosia::render::text::draw_aligned(ImGui::GetWindowDrawList(), bb.Min + style.FramePadding, bb.Max - style.FramePadding, style.ButtonTextAlign, celosia::style::themes::active.text, label, celosia::render::text::label_end(label), &label_size, &bb);
}

bool celosia::widgets::button(const char* label, const ImVec2& size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = celosia::render::text::size(label, celosia::render::text::label_end(label));

    const ImVec2 pos = window->DC.CursorPos;
    const ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    const float glow = celosia::animations::set(celosia::animations::key("button_glow", id), hovered ? 1.f : 0.f);
    if (glow > 0.01f) { // hovered buttons glow faintly in the accent
        const ImVec4 main = celosia::style::general::accent.Value;
        celosia::effects::glow(window->DrawList, bb.Min, bb.Max, ImColor(main.x, main.y, main.z, 0.35f * glow), style.FrameRounding, 8.f * celosia::style::scale);
    }

    const ImColor col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImGui::RenderNavHighlight(bb, id);
    style1(id, bb, label, label_size, style, col);

    return pressed;
}
