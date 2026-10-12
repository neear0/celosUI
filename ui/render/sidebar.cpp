#include "../ui.h"

void celosia::render::sidebar() {
	ImDrawList* drawlist_window = ImGui::GetWindowDrawList();
	render::panel(drawlist_window, ImVec2(0, style::titlebar::height), ImVec2(style::sidebar::width, ui::size.y), style::themes::active.background_darker, (float)style::general::rounding_window);
	ImGui::SetCursorPos(ImVec2(style::general::padding.x, style::titlebar::height));
	ImGui::BeginChild("Sidebar", ImVec2(style::sidebar::width - style::general::padding.x, ui::size.y - style::titlebar::height));
	
	render::tabs({ "tab1", "tab2" });

	if (ImGui::Button("switch layout", ImVec2(celosia::style::sidebar::width - celosia::style::general::padding.x * 2.f, 40))) {

		if (style::themes::active.tab_style == style::themes::e_tab_title_style::full)
			style::themes::active.tab_style = style::themes::e_tab_title_style::minimal;
		else
			style::themes::active.tab_style = style::themes::e_tab_title_style::full;

	}

	if (platform::blur_supported() && ImGui::Button("switch background", ImVec2(celosia::style::sidebar::width - celosia::style::general::padding.x * 2.f, 40))) {
		style::window::background = style::window::background == style::window::blur ? style::window::shader : style::window::blur;
		platform::set_blur(style::window::background == style::window::blur);
	}

	if (ImGui::Button("switch noise", ImVec2(celosia::style::sidebar::width - celosia::style::general::padding.x * 2.f, 40)))
		style::window::noise = style::window::noise > 0.f ? 0.f : 0.04f;

	if (ImGui::Button("switch theme", ImVec2(celosia::style::sidebar::width - celosia::style::general::padding.x * 2.f, 40)))
		style::themes::set(style::themes::active.name == "dark" ? style::themes::light : style::themes::dark);

	ImGui::EndChild();
}
