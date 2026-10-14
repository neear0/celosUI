#include "../ui.h"

static const ImGuiID active_spacing = celosia::animations::key("groupbox_active_spacing");
static const ImGuiID text_color_key = celosia::animations::key("Groupbox Text Color");

// where the groupbox at groupbox::index goes in the space right of the sidebar, moved down by the tab switch animation:
// fullscreen takes all of it, two share the height, four make a 2x2 grid filled row by row
static void rect(celosia::render::groupbox::e_group_layout layout, ImVec2* offset, ImVec2* offset_max, ImVec2* offset_size) {
	using celosia::render::groupbox::e_group_layout;
	const ImVec2 padding = celosia::style::frame::size_padding;
	const float title = celosia::style::titlebar::height;
	const int columns = layout == e_group_layout::four ? 2 : 1;
	const int rows = layout == e_group_layout::fullscreen ? 1 : 2;
	const int cell = celosia::render::groupbox::index % (columns * rows);
	const float left = celosia::style::sidebar::width + padding.x;
	const float width = (celosia::ui::size.x - padding.x - left) / columns;
	const float height = ((celosia::ui::size.y - title) / rows) - padding.y;

	*offset = {
		left + width * (cell % columns),
		title + celosia::animations::get(active_spacing) + padding.y + (height * (cell / columns))
	};
	*offset_max = {
		offset->x + width - (cell % columns < columns - 1 ? padding.x : 0.f), // a gap between columns, the last one ends at the window's padding
		offset->y + height - padding.y
	};

	*offset_size = {
		offset_max->x - offset->x,
		offset_max->y - offset->y
	};
}

float celosia::render::groupbox::title_height(const char* description) { // fits the title and description with padding around them, never below style::groupbox::height
	float text = render::text::calc::font("a", resources::fonts::get("default")).y;
	if (description && *description)
		text += render::text::calc::font("a", resources::fonts::get("default_smaller")).y;
	return ImMax(style::groupbox::height, text + style::groupbox::padding * 2.f);
}

void celosia::render::groupbox::body(ImDrawList* drawlist, const ImVec2 offset, const ImVec2 offset_max, const char* title, const char* description) {
	const float height = title_height(description);
	ImColor a = style::general::main_color;
	ImColor b = style::themes::rgb_accent(a);
	ImColor text_color = style::themes::active.text;

	render::panel(drawlist, ImVec2(offset.x, offset.y + height / 2), offset_max, style::themes::active.background_darker, (float)style::general::rounding);

	switch (style::themes::active.tab_style) {
	case style::themes::e_tab_title_style::minimal:
		celosia::widgets::add_rect_filled_half_rounded(drawlist, offset, ImVec2(offset_max.x, offset.y + height), style::themes::active.frame_fill, 4);
		effects::gradient(drawlist, ImVec2(offset.x, offset.y + height - 2), ImVec2(offset_max.x, offset.y + height), a, b); // underline
		break;
	case style::themes::e_tab_title_style::full:
		effects::gradient(drawlist, offset, ImVec2(offset_max.x, offset.y + height), a, b, (float)style::general::rounding, 0.f, 0.15f); // slowly slides back and forth

		const float combined_rgb = style::general::main_color.Value.x + style::general::main_color.Value.y + style::general::main_color.Value.z; // could be made into a function, i don't see the point though because i don't plan on using this any other place
		float text_color_goal;

		if (combined_rgb > 1.5f)
			text_color_goal = 0.15f;
		else
			text_color_goal = 1.f;


		const float text_color_fl = animations::set(text_color_key, text_color_goal, 1.f);
		text_color = ImColor(text_color_fl, text_color_fl, text_color_fl, style::themes::active.text.Value.w);

		break;
	}
	
	ImFont* font_default = resources::fonts::get("default");
	ImFont* font_default_smaller = resources::fonts::get("default_smaller");

	const float text_x = offset.x + style::frame::size_padding.x;
	const float center_y = offset.y + height / 2;
	const float title_text_height = render::text::calc::font("a", font_default).y;
	const float description_height = render::text::calc::font("a", font_default_smaller).y;

	if (description && *description) {
		const float top = center_y - (title_text_height + description_height) / 2; // title and description stacked, centered together
		render::text::font(drawlist, title, ImVec2(text_x, top), text_color, font_default);
		render::text::font(drawlist, description, ImVec2(text_x, top + title_text_height), text_color, font_default_smaller);
	}
	else
		render::text::font(drawlist, title, ImVec2(text_x, center_y - title_text_height / 2), text_color, font_default);
}

void celosia::render::groupbox::begin(const char* title, const char* description, e_group_layout layout) {
	ImDrawList* drawlist_window = ImGui::GetWindowDrawList();
	ImVec2 offset, offset_max, offset_size;
	rect(layout, &offset, &offset_max, &offset_size);
	const float height = title_height(description);

	body(drawlist_window, offset, offset_max, title, description);
	ImGui::SetCursorPos(ImVec2(offset.x + style::frame::size_padding.x, offset.y + style::frame::size_padding.y + height));
	ImGui::BeginChild(title, ImVec2(offset_size.x - style::frame::size_padding.x, offset_size.y - height - (2 * style::frame::size_padding.y)));
	tab_content.children.push_back(ImGui::GetWindowDrawList());
}

void celosia::render::groupbox::end() {
	index += 1;
	ImGui::EndChild();
}
