#include "../../ui.h"

// text(): a line of text as an item, in the current font and the theme's text color

void celosia::widgets::text(const char* text) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImVec2 pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset); // on the line's baseline
    const ImVec2 size = celosia::render::text::size(text);
    ImGui::ItemSize(size, 0.f);
    if (!ImGui::ItemAdd(ImRect(pos, pos + size), 0))
        return;
    celosia::render::text::draw(window->DrawList, pos, celosia::style::themes::active.text, text);
}
