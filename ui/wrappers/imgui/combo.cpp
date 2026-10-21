#include "../../ui.h"

// combo(): a frame showing the picked item with an arrow, opening a frosted popup under it (above when there's no room
// below) that lists the items, at most max_items rows of them before it scrolls

static float popup_height(int items) { // of a popup that shows that many items
    const ImGuiStyle& style = ImGui::GetStyle();
    return (ImGui::GetFontSize() + style.ItemSpacing.y) * items - style.ItemSpacing.y + style.WindowPadding.y * 2.f;
}

static bool selectable(const char* label, bool selected) { // an item in the popup, as wide as it, highlighted when picked or hovered
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = celosia::render::text::size(label, celosia::render::text::label_end(label));
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(label_size, 0.f);

    // the text stays where it is, the box spans the rest of the line and the spacing around it, the items meet without a gap to click into
    const ImVec2 text_min = pos, text_max(pos.x + ImMax(label_size.x, window->WorkRect.Max.x - pos.x), pos.y + label_size.y);
    ImRect bb(text_min, text_max);
    const float spacing_left = IM_TRUNC(style.ItemSpacing.x * 0.5f), spacing_up = IM_TRUNC(style.ItemSpacing.y * 0.5f);
    bb.Min -= ImVec2(spacing_left, spacing_up);
    bb.Max += ImVec2(style.ItemSpacing.x - spacing_left, style.ItemSpacing.y - spacing_up);
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

    ImColor col = ImGui::GetColorU32(ImGuiCol_WindowBg);
    if (selected) col = ImGui::GetColorU32(ImGuiCol_Header);
    if (hovered) col = ImGui::GetColorU32(ImGuiCol_HeaderHovered);
    if (pressed || held) col = ImGui::GetColorU32(ImGuiCol_HeaderActive);
    celosia::widgets::render_frame_animated(id, bb.Min, bb.Max, col);
    if (g.NavId == id)
        ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    celosia::render::text::draw_aligned(window->DrawList, text_min, text_max, style.SelectableTextAlign, celosia::style::themes::active.text, label, celosia::render::text::label_end(label), &label_size, &bb);

    if (pressed && (window->Flags & ImGuiWindowFlags_Popup))
        ImGui::CloseCurrentPopup();
    return pressed;
}

static bool begin_popup(ImGuiID popup_id, const ImRect& frame, int max_items) { // the popup's window, true while it's open
    ImGuiContext& g = *GImGui;
    if (!ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None))
        return false;

    ImGui::SetNextWindowSizeConstraints(ImVec2(frame.GetWidth(), 0.f), ImVec2(FLT_MAX, popup_height(max_items))); // at least as wide as the frame

    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // one window per popup depth, reused
    if (ImGuiWindow* popup_window = ImGui::FindWindowByName(name))
        if (popup_window->WasActive) { // placed from the size it's about to have: below the frame, or above it when it doesn't fit
            const ImVec2 size_expected = ImGui::CalcWindowNextAutoFitSize(popup_window);
            popup_window->AutoPosLastDirection = ImGuiDir_Down;
            const ImRect r_outer = ImGui::GetPopupAllowedExtentRect(popup_window);
            ImGui::SetNextWindowPos(ImGui::FindBestWindowPosForPopupEx(frame.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame, ImGuiPopupPositionPolicy_ComboBox));
        }

    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(g.Style.FramePadding.x, g.Style.WindowPadding.y)); // the items' text lines up with the frame's
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(0, 0, 0, 0)); // the blur below is the background, Begin() would draw an opaque one first
    const bool open = ImGui::Begin(name, nullptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    if (!open) {
        ImGui::EndPopup();
        return false;
    }

    ImGuiWindow* popup = ImGui::GetCurrentWindow(); // frosted glass over whatever is behind the popup
    ImColor tint = celosia::style::themes::active.background_darker;
    tint.Value.w = 0.6f;
    popup->DrawList->PushClipRect(popup->Pos, popup->Pos + popup->Size);
    celosia::effects::blur(popup->DrawList, popup->Pos, popup->Pos + popup->Size, tint, g.Style.PopupRounding, celosia::style::window::glass_blur);
    celosia::effects::noise(popup->DrawList, popup->Pos, popup->Pos + popup->Size, celosia::style::window::noise, g.Style.PopupRounding);
    popup->DrawList->PopClipRect();
    return true;
}

bool celosia::widgets::combo(const char* label, int* current, const char* const items[], int count, int max_items) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID id = window->GetID(label);
    const float arrow_size = ImGui::GetFrameHeight();
    const ImVec2 label_size = render::text::size(label, render::text::label_end(label));
    const ImRect frame(window->DC.CursorPos, window->DC.CursorPos + ImVec2(ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y * 2.f));
    const ImRect total(frame.Min, frame.Max + ImVec2(label_size.x > 0.f ? style.ItemInnerSpacing.x + label_size.x : 0.f, 0.f));
    ImGui::ItemSize(total, style.FramePadding.y);
    if (!ImGui::ItemAdd(total, id, &frame))
        return false;

    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(frame, id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if (pressed && !popup_open) {
        ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    // outlined like a button, the outline and the arrow in the accent while it's open
    const ImVec4 text = style::themes::active.text.Value;
    ImVec4 border = ImLerp(ImGui::GetStyleColorVec4(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), text, 0.12f);
    border.w = ImGui::GetStyleColorVec4(ImGuiCol_Button).w;
    ImVec4 arrow = ImVec4(text.x, text.y, text.z, 0.7f);
    if (popup_open) border = arrow = style::general::accent.Value;
    if (held) border = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

    const float value_x2 = ImMax(frame.Min.x, frame.Max.x - arrow_size);
    ImGui::RenderNavHighlight(frame, id);
    window->DrawList->AddRectFilled(frame.Min, ImVec2(value_x2, frame.Max.y), ImGui::GetColorU32(ImGuiCol_Button), style.FrameRounding, ImDrawFlags_RoundCornersLeft);
    if (value_x2 + arrow_size - style.FramePadding.x <= frame.Max.x)
        ImGui::RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame.Min.y + style.FramePadding.y), ImGui::GetColorU32(arrow), ImGuiDir_Down, 1.f);
    render_frame_border_animated(id, frame.Min, frame.Max, ImGui::GetColorU32(border), style.FrameRounding);

    if (*current >= 0 && *current < count)
        render::text::draw_aligned(window->DrawList, frame.Min + style.FramePadding, ImVec2(value_x2, frame.Max.y), ImVec2(0.f, 0.f), style::themes::active.text, items[*current], render::text::label_end(items[*current]));
    if (label_size.x > 0.f)
        render::text::draw(window->DrawList, ImVec2(frame.Max.x + style.ItemInnerSpacing.x, frame.Min.y + style.FramePadding.y), style::themes::active.text, label, render::text::label_end(label));

    if (!popup_open || !begin_popup(popup_id, frame, max_items > 0 ? max_items : 8))
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
    ImGui::EndPopup();

    if (changed)
        ImGui::MarkItemEdited(id);
    return changed;
}
