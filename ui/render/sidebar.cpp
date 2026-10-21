#include "../ui.h"

void celosia::render::sidebar() {
	ImDrawList* drawlist_window = ImGui::GetWindowDrawList();
	render::panel(drawlist_window, ImVec2(0, style::titlebar::height), ImVec2(style::sidebar::width, ui::size.y), style::themes::active.background_darker, (float)style::general::rounding_window);
	ImGui::SetCursorPos(ImVec2(style::general::padding.x, style::titlebar::height));
	ImGui::BeginChild("Sidebar", ImVec2(style::sidebar::width - style::general::padding.x, ui::size.y - style::titlebar::height));
	
	render::tabs({ "Widgets", "Appearance" });

	ImGui::EndChild();
}
