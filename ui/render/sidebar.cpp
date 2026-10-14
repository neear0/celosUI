#include "../ui.h"

void celosia::render::sidebar() {
	ImDrawList* drawlist_window = ImGui::GetWindowDrawList();
	render::panel(drawlist_window, ImVec2(0, style::titlebar::height), ImVec2(style::sidebar::width, ui::size.y), style::themes::active.background_darker, (float)style::general::rounding_window);
	ImGui::SetCursorPos(ImVec2(style::general::padding.x, style::titlebar::height));
	ImGui::BeginChild("Sidebar", ImVec2(style::sidebar::width - style::general::padding.x, ui::size.y - style::titlebar::height));
	
	render::tabs({ "tab1", "tab2" });

	const ImVec2 button_size = ImVec2(style::sidebar::width - style::general::padding.x * 2.f, 40.f * style::scale);
	if (ImGui::Button("switch layout", button_size))
		style::groupbox::title_style = style::groupbox::title_style == style::groupbox::full ? style::groupbox::minimal : style::groupbox::full;

	if (platform::blur_supported() && ImGui::Button("switch background", button_size)) {
		style::window::background = style::window::background == style::window::blur ? style::window::shader : style::window::blur;
		platform::set_blur(style::window::background == style::window::blur);
	}

	if (ImGui::Button("switch noise", button_size))
		style::window::noise = style::window::noise > 0.f ? 0.f : 0.04f;

	if (ImGui::Button("switch theme", button_size))
		style::themes::set(style::themes::active.name == "dark" ? style::themes::light : style::themes::dark);

	ImGui::EndChild();
}
