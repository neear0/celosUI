#include "../../ui.h"

// segmented(): side by side choices sharing one frame as wide as a slider, with the label after it. a pill in the accent
// slides to the picked one and fades out while nothing is picked (current -1). text over the pill is dark or white,
// whichever stands out more from the accent

bool celosia::widgets::segmented(const char* label, int* current, const char* const items[], int count) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems || count <= 0)
        return false;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect frame(pos, pos + ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight()));
    const ImRect total(pos, frame.Max + ImVec2(label_size.x > 0.f ? style.ItemInnerSpacing.x + label_size.x : 0.f, 0.f));
    ImGui::ItemSize(total, style.FramePadding.y);
    if (!ImGui::ItemAdd(total, id))
        return false;

    ImDrawList* drawlist = window->DrawList;
    const ImVec4 text = celosia::style::themes::active.text.Value;
    const ImVec4 accent = celosia::style::general::accent.Value;
    const float segment = frame.GetWidth() / (float)count;
    const float scale = celosia::style::scale;

    // the frame like a button's: the idle color with an outline a little towards the text color
    const ImVec4 fill = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    ImVec4 border = ImLerp(fill, text, 0.12f);
    border.w = fill.w;
    drawlist->AddRectFilled(frame.Min, frame.Max, ImGui::GetColorU32(fill), style.FrameRounding);
    drawlist->AddRect(frame.Min, frame.Max, ImGui::GetColorU32(border), style.FrameRounding, 0, celosia::style::frame::border_size);

    bool changed = false;
    for (int i = 0; i < count; i++) {
        const ImRect rect(ImVec2(frame.Min.x + segment * i, frame.Min.y), ImVec2(frame.Min.x + segment * (i + 1), frame.Max.y));
        ImGui::PushID(i);
        const ImGuiID item = window->GetID("##segment");
        ImGui::PopID();
        if (!ImGui::ItemAdd(rect, item))
            continue;
        bool hovered, held;
        if (ImGui::ButtonBehavior(rect, item, &hovered, &held) && *current != i) {
            *current = i;
            changed = true;
            ImGui::MarkItemEdited(id);
        }
        const float hover = celosia::animations::set(celosia::animations::key("segmented_hover", item), hovered && *current != i ? 1.f : 0.f, celosia::animations::color_speed);
        if (hover > 0.01f)
            drawlist->AddRectFilled(rect.Min, rect.Max, ImColor(text.x, text.y, text.z, 0.08f * hover), style.FrameRounding);
    }

    // the pill keeps its place while nothing is picked, so it fades out where it was. a hidden one appears where it's
    // going instead of sliding in from wherever it was last
    const ImGuiID pill_key = celosia::animations::key("segmented_pill", id), alpha_key = celosia::animations::key("segmented_pill_alpha", id);
    const float pill_goal = segment * *current; // from the frame's left edge, so it doesn't trail behind when the groupbox scrolls
    if (*current >= 0 && celosia::animations::get(alpha_key) < 0.01f)
        celosia::animations::snap(pill_key, pill_goal);
    const float pill_x = celosia::animations::set(pill_key, *current >= 0 ? pill_goal : celosia::animations::get(pill_key), 14.f);
    const float pill_alpha = celosia::animations::set(alpha_key, *current >= 0 ? 1.f : 0.f, celosia::animations::color_speed);
    const float inset = 3.f * scale;
    if (pill_alpha > 0.01f) {
        const ImVec2 pill_min(frame.Min.x + pill_x + inset, frame.Min.y + inset), pill_max(frame.Min.x + pill_x + segment - inset, frame.Max.y - inset);
        celosia::effects::glow(drawlist, pill_min, pill_max, ImColor(accent.x, accent.y, accent.z, 0.35f * pill_alpha), style.FrameRounding, 6.f * scale);
        drawlist->AddRectFilled(pill_min, pill_max, ImColor(accent.x, accent.y, accent.z, pill_alpha), ImMax(style.FrameRounding - inset * 0.5f, 0.f));
    }

    const ImVec4 on_accent = celosia::style::themes::text_on(accent).Value;
    for (int i = 0; i < count; i++) { // each label turns to the text on the accent as far as the pill covers its segment
        const float x = frame.Min.x + segment * i;
        const float covered = ImSaturate(1.f - fabsf(pill_x - segment * i) / segment) * pill_alpha;
        const ImVec2 size = ImGui::CalcTextSize(items[i]);
        const ImVec2 at(x + (segment - size.x) * 0.5f, frame.GetCenter().y - size.y * 0.5f);
        drawlist->PushClipRect(ImVec2(x, frame.Min.y), ImVec2(x + segment, frame.Max.y), true);
        drawlist->AddText(at, ImGui::GetColorU32(ImLerp(text, on_accent, covered)), items[i]);
        drawlist->PopClipRect();
    }
    if (label_size.x > 0.f)
        ImGui::RenderText(ImVec2(frame.Max.x + style.ItemInnerSpacing.x, frame.GetCenter().y - label_size.y * 0.5f), label);

    return changed;
}
