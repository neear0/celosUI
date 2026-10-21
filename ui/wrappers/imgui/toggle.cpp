#include "../../ui.h"

// toggle(): an on/off switch, a rounded track with a knob that slides across, the label after it. off, the track is faint
// in the text color like a slider's; on, it fills in the accent and the knob takes the text color that stands out on it

bool celosia::widgets::toggle(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
    const float frame_height = ImGui::GetFrameHeight();
    const float height = IM_TRUNC(frame_height * 0.6f), width = IM_TRUNC(height * 1.8f);
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total(pos, pos + ImVec2(width + (label_size.x > 0.f ? style.ItemInnerSpacing.x + label_size.x : 0.f), frame_height));
    ImGui::ItemSize(total, style.FramePadding.y);
    if (!ImGui::ItemAdd(total, id))
        return false;

    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(total, id, &hovered, &held);
    if (pressed) {
        *v = !*v;
        ImGui::MarkItemEdited(id);
    }

    ImDrawList* drawlist = window->DrawList;
    const ImVec4 text = celosia::style::themes::active.text.Value;
    const ImVec4 accent = celosia::style::general::accent.Value;
    const float scale = celosia::style::scale;
    const ImRect track(ImVec2(pos.x, pos.y + (frame_height - height) * 0.5f), ImVec2(pos.x + width, pos.y + (frame_height + height) * 0.5f));
    const float radius = height * 0.5f;

    const float on = celosia::animations::set(celosia::animations::key("toggle_on", id), *v ? 1.f : 0.f, 12.f);
    const float hover = celosia::animations::set(celosia::animations::key("toggle_hover", id), hovered ? 1.f : 0.f, celosia::animations::color_speed);
    const ImVec4 off_track(text.x, text.y, text.z, 0.12f + 0.1f * hover);
    if (on > 0.01f) // glows in the accent while it's on, a little more when hovered
        celosia::effects::glow(drawlist, track.Min, track.Max, ImColor(accent.x, accent.y, accent.z, (0.25f + 0.15f * hover) * on), radius, 8.f * scale);
    drawlist->AddRectFilled(track.Min, track.Max, ImGui::GetColorU32(ImLerp(off_track, ImVec4(accent.x, accent.y, accent.z, 1.f), on)), radius);

    const float knob_radius = radius - 3.f * scale;
    const ImVec2 knob(ImLerp(track.Min.x + radius, track.Max.x - radius, on), track.GetCenter().y);
    const ImVec4 knob_color = ImLerp(ImVec4(text.x, text.y, text.z, 0.9f), celosia::style::themes::text_on(accent).Value, on);
    drawlist->AddCircleFilled(knob, knob_radius * (held ? 0.85f : 1.f), ImGui::GetColorU32(knob_color));

    if (label_size.x > 0.f)
        ImGui::RenderText(ImVec2(track.Max.x + style.ItemInnerSpacing.x, pos.y + (frame_height - label_size.y) * 0.5f), label);

    return pressed;
}
