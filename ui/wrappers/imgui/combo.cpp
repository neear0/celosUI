#include "../../ui.h"

// combo(): a frame showing the picked item with an arrow, opening a frosted popup under it (above when there's no room
// below) that lists the items, at most max_items rows of them before it scrolls

static float popup_height(int items, float padding) { // of a popup that shows that many items, padding above and below them
    const float spacing = celosia::style::general::spacing.y;
    return (ImGui::GetFontSize() + spacing) * items - spacing + padding * 2.f;
}

static bool selectable(const char* label, bool selected) { // an item in the popup, as wide as it, highlighted when picked or hovered
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImVec2 spacing = celosia::style::general::spacing;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = celosia::render::text::size(label, celosia::render::text::label_end(label));
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(label_size, 0.f);

    // the text stays where it is, the box spans the rest of the line and the spacing around it, the items meet without a gap to click into
    const ImVec2 text_min = pos, text_max(pos.x + ImMax(label_size.x, window->WorkRect.Max.x - pos.x), pos.y + label_size.y);
    ImRect bb(text_min, text_max);
    const float spacing_left = IM_TRUNC(spacing.x * 0.5f), spacing_up = IM_TRUNC(spacing.y * 0.5f);
    bb.Min -= ImVec2(spacing_left, spacing_up);
    bb.Max += ImVec2(spacing.x - spacing_left, spacing.y - spacing_up);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    if (pressed) {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent) { // the keyboard carries on from the item clicked
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, ImGui::WindowRectAbsToRel(window, bb));
            g.NavDisableHighlight = true;
        }
        ImGui::MarkItemEdited(id);
    }

    ImColor col = celosia::style::window::background_color();
    if (selected) col = celosia::style::general::accent;
    if (hovered) col = celosia::style::themes::active.hovered;
    if (pressed || held) col = celosia::style::themes::rgb_intensity(celosia::style::general::accent, 0.5f);
    celosia::widgets::render_frame_animated(id, bb.Min, bb.Max, col);
    if (g.NavId == id)
        ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    celosia::render::text::draw_aligned(window->DrawList, text_min, text_max, ImVec2(0.f, 0.f), celosia::style::themes::active.text, label, celosia::render::text::label_end(label), &label_size, &bb);

    if (pressed && (window->Flags & ImGuiWindowFlags_Popup))
        celosia::popup::close_current();
    return pressed;
}

bool celosia::widgets::combo(const char* label, int* current, const char* const items[], int count, int max_items) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImVec2 padding = style::frame::size_padding, inner = style::general::inner_spacing;
    const float rounding = (float)style::general::rounding;
    const ImGuiID id = window->GetID(label);
    const float arrow_size = ImGui::GetFrameHeight();
    const ImVec2 label_size = render::text::size(label, render::text::label_end(label));
    const ImRect frame(window->DC.CursorPos, window->DC.CursorPos + ImVec2(ImGui::CalcItemWidth(), label_size.y + padding.y * 2.f));
    const ImRect total(frame.Min, frame.Max + ImVec2(label_size.x > 0.f ? inner.x + label_size.x : 0.f, 0.f));
    ImGui::ItemSize(total, padding.y);
    if (!ImGui::ItemAdd(total, id, &frame))
        return false;

    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(frame, id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = popup::is_open(popup_id);
    if (pressed && !popup_open) {
        popup::open(popup_id);
        popup_open = true;
    }

    // outlined like a button, the outline and the arrow in the accent while it's open
    const style::themes::t_theme& theme = style::themes::active;
    const ImVec4 text = theme.text.Value;
    ImVec4 border = ImLerp(hovered ? theme.hovered.Value : theme.idle.Value, text, 0.12f);
    border.w = theme.idle.Value.w;
    ImVec4 arrow = ImVec4(text.x, text.y, text.z, 0.7f);
    if (popup_open) border = arrow = style::general::accent.Value;
    if (held) border = theme.held.Value;

    const float value_x2 = ImMax(frame.Min.x, frame.Max.x - arrow_size);
    ImGui::RenderNavHighlight(frame, id);
    window->DrawList->AddRectFilled(frame.Min, ImVec2(value_x2, frame.Max.y), theme.idle, rounding, ImDrawFlags_RoundCornersLeft);
    if (value_x2 + arrow_size - padding.x <= frame.Max.x)
        ImGui::RenderArrow(window->DrawList, ImVec2(value_x2 + padding.y, frame.Min.y + padding.y), ImGui::GetColorU32(arrow), ImGuiDir_Down, 1.f);
    render_frame_border_animated(id, frame.Min, frame.Max, ImGui::GetColorU32(border), rounding);

    if (*current >= 0 && *current < count)
        render::text::draw_aligned(window->DrawList, frame.Min + padding, ImVec2(value_x2, frame.Max.y), ImVec2(0.f, 0.f), style::themes::active.text, items[*current], render::text::label_end(items[*current]));
    if (label_size.x > 0.f)
        render::text::draw(window->DrawList, ImVec2(frame.Max.x + inner.x, frame.Min.y + padding.y), style::themes::active.text, label, render::text::label_end(label));

    // at least as wide as the frame, with the items' text lined up with the frame's
    const ImVec2 popup_padding(padding.x, 0.f);
    if (!popup_open || !popup::begin(popup_id, frame, popup_padding, frame.GetWidth(), popup_height(max_items > 0 ? max_items : 8, popup_padding.y)))
        return false;

    bool changed = false;
    for (int i = 0; i < count; i++) {
        ImGui::PushID(i);
        const bool picked = i == *current;
        if (selectable(items[i], picked) && !picked) {
            *current = i;
            changed = true;
        }
        if (picked)
            ImGui::SetItemDefaultFocus(); // the keyboard starts at the picked item
        ImGui::PopID();
    }
    popup::end();

    if (changed)
        ImGui::MarkItemEdited(id);
    return changed;
}
