#include "../../ui.h"

// the surfaces widgets go on, ImGui's windows underneath: the window the ui fills, scroll areas inside it and popups
// over it. everything else goes through these, so they're the one place that knows about ImGui's windows

void celosia::window::begin(const char* name, ImVec2 pos, ImVec2 size) {
    ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    ImGui::SetWindowSize(size);
    ImGui::SetWindowPos(pos);
}

void celosia::window::end() {
    ImGui::End();
}

void celosia::scroll_area::begin(ImGuiID id, ImVec2 size) {
    ImGui::BeginChild(id, size);
}

void celosia::scroll_area::begin(const char* name, ImVec2 size) {
    ImGui::BeginChild(name, size);
}

void celosia::scroll_area::end() {
    ImGui::EndChild();
}

void celosia::popup::open(ImGuiID id) {
    ImGui::OpenPopupEx(id, ImGuiPopupFlags_None);
}

bool celosia::popup::is_open(ImGuiID id) {
    return ImGui::IsPopupOpen(id, ImGuiPopupFlags_None);
}

bool celosia::popup::begin(ImGuiID id, const ImRect& anchor, ImVec2 padding, float min_width, float max_height) {
    if (!is_open(id))
        return false;

    if (min_width > 0.f || max_height < FLT_MAX)
        ImGui::SetNextWindowSizeConstraints(ImVec2(min_width, 0.f), ImVec2(FLT_MAX, max_height));

    // under the anchor, or above it when it doesn't fit there, worked out from the size it's about to have. the window
    // is the one BeginPopupEx() names after the id
    char name[20];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Popup_%08x", id);
    ImGuiWindow* existing = ImGui::FindWindowByName(name);
    if (existing && existing->WasActive) {
        existing->AutoPosLastDirection = ImGuiDir_Down;
        ImGui::SetNextWindowPos(ImGui::FindBestWindowPosForPopupEx(anchor.GetBL(), ImGui::CalcWindowNextAutoFitSize(existing), &existing->AutoPosLastDirection,
            ImGui::GetPopupAllowedExtentRect(existing), anchor, ImGuiPopupPositionPolicy_ComboBox));
    }
    else
        ImGui::SetNextWindowPos(anchor.GetBL());

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(0, 0, 0, 0)); // the frosted glass below is the background, Begin() would draw an opaque one first
    const bool open = ImGui::BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    if (!open)
        return false;

    // frosted glass over whatever is behind the popup. ImGui clips a window's contents half its padding in from its edges,
    // the glass goes all the way to them
    ImGuiWindow* popup = ImGui::GetCurrentWindow();
    const float rounding = (float)style::general::rounding_popup;
    ImColor tint = style::themes::active.background_darker;
    tint.Value.w = 0.6f;
    popup->DrawList->PushClipRect(popup->Pos, popup->Pos + popup->Size);
    effects::blur(popup->DrawList, popup->Pos, popup->Pos + popup->Size, tint, rounding, style::window::glass_blur);
    effects::noise(popup->DrawList, popup->Pos, popup->Pos + popup->Size, style::window::noise, rounding);
    popup->DrawList->PopClipRect();
    return true;
}

void celosia::popup::end() {
    ImGui::EndPopup();
}

void celosia::popup::close_current() {
    ImGui::CloseCurrentPopup();
}
