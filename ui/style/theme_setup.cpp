#include "../ui.h"

static celosia::style::themes::t_theme target; // what set() was last given, active eases towards it

void celosia::style::themes::initialize() {
	dark.name = "dark";
	dark.text = ImColor(255, 255, 255, 255);
	dark.background = ImColor(12, 12, 12, 180);
	dark.background_darker = ImColor(10, 10, 10, 200);
	dark.idle = ImColor(18, 18, 18, 255);
	dark.hovered = ImColor(64, 64, 64, 255);
	dark.held = ImColor(16, 16, 16, 255);
	dark.active = ImColor(28, 28, 28, 255);
	dark.frame_border = ImColor(0, 0, 0, 0);
	dark.frame_fill = ImColor(16, 16, 16, 255);
	dark.tab_style = e_tab_title_style::full;

	light.name = "light";
	light.text = ImColor(18, 18, 18, 255);
	light.background = ImColor(242, 242, 245, 190);
	light.background_darker = ImColor(226, 226, 230, 210);
	light.idle = ImColor(252, 252, 252, 255);
	light.hovered = ImColor(205, 205, 212, 255);
	light.held = ImColor(190, 190, 198, 255);
	light.active = ImColor(235, 235, 240, 255);
	light.frame_border = ImColor(0, 0, 0, 0);
	light.frame_fill = ImColor(236, 236, 240, 255);
	light.tab_style = e_tab_title_style::full;
}

void celosia::style::themes::set(const t_theme& theme, bool instant) {
	target = theme;
	active.name = theme.name;
	active.tab_style = theme.tab_style;
	if (instant)
		active = theme;
}

void celosia::style::themes::update() {
	static constexpr ImColor t_theme::* colors[] = { // the members that ease, the rest switch at once
		&t_theme::background, &t_theme::background_darker, &t_theme::frame_border, &t_theme::frame_fill,
		&t_theme::idle, &t_theme::held, &t_theme::hovered, &t_theme::active, &t_theme::text,
	};
	const float t = 1.f - expf(-animations::delta_time * lerp_speed); // the same share of the way each second, whatever the frame rate
	for (ImColor t_theme::* color : colors) {
		const ImVec4 from = (active.*color).Value, to = (target.*color).Value;
		(active.*color).Value = ImLerp(from, to, t);
	}
	apply();
}

void celosia::style::apply() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = (float)general::rounding;
	style.FramePadding = frame::size_padding;
	style.WindowRounding = (float)general::rounding_window;
	style.WindowPadding = ImVec2(0, 0);
	style.ScrollbarSize = 4.f;
	style.ItemSpacing = general::spacing;

	const themes::t_theme& theme = themes::active;
	ImVec4* c = style.Colors;
	c[ImGuiCol_Text] = theme.text;
	c[ImGuiCol_WindowBg] = theme.background;
	if (window::background == window::blur) // the theme's alpha alone hides most of the blurred desktop
		c[ImGuiCol_WindowBg].w *= window::glass_opacity;

	c[ImGuiCol_FrameBg] = theme.idle;
	c[ImGuiCol_FrameBgHovered] = theme.hovered;
	c[ImGuiCol_FrameBgActive] = theme.held;

	c[ImGuiCol_Button] = theme.idle;
	c[ImGuiCol_ButtonHovered] = theme.hovered;
	c[ImGuiCol_ButtonActive] = theme.held;

	c[ImGuiCol_HeaderHovered] = theme.hovered;
	c[ImGuiCol_Header] = general::main_color;
	c[ImGuiCol_HeaderActive] = themes::rgb_intensity(general::main_color, 0.5f);

	c[ImGuiCol_CheckMark] = theme.idle;
	c[ImGuiCol_ScrollbarBg] = theme.background_darker;
	c[ImGuiCol_ScrollbarGrab] = theme.idle;
	c[ImGuiCol_Border] = theme.frame_border;

	c[ImGuiCol_TitleBg] = theme.idle;
	c[ImGuiCol_TitleBgActive] = theme.idle;
	c[ImGuiCol_TitleBgCollapsed] = theme.idle;
}
