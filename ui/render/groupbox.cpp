#include "../ui.h"

namespace celosia::render::groupbox {
	static const ImGuiID active_spacing = animations::key("groupbox_active_spacing");
	static const ImGuiID text_color_key = animations::key("Groupbox Text Color");

	void return_full(ImVec2* offset, ImVec2* offset_max, ImVec2* offset_size) {
		*offset = {
			style::sidebar::width + style::frame::size_padding.x,
			style::titlebar::height + animations::get(active_spacing) + style::frame::size_padding.y
		};
		*offset_max = {
			ui::size.x - style::frame::size_padding.x,
			offset->y + ((ui::size.y - style::titlebar::height)) - style::frame::size_padding.y - style::frame::size_padding.y
		};

		*offset_size = {
			offset_max->x - offset->x,
			offset_max->y - offset->y
		};
	}

	void return_two(ImVec2* offset, ImVec2* offset_max, ImVec2* offset_size) {
		float size = ((ui::size.y - style::titlebar::height) / 2) - style::frame::size_padding.y;

		*offset = {
			style::sidebar::width + style::frame::size_padding.x,
			style::titlebar::height + animations::get(active_spacing) + style::frame::size_padding.y + (size * index)
		};
		*offset_max = {
			ui::size.x - style::frame::size_padding.x, 
			offset->y + size - style::frame::size_padding.y
		};

		*offset_size = {
			offset_max->x - offset->x, 
			offset_max->y - offset->y
		};
	}

	void body(ImDrawList* drawlist, const ImVec2 offset, const ImVec2 offset_max, const char* title, const char* description, e_group_layout layout) { // ctodo: groupbox height should be text_size.y + padding * 2
		ImColor a = style::general::main_color;
		ImColor b = style::themes::rgb_accent(a);
		ImColor text_color = style::themes::active.text;
		
		render::panel(drawlist, ImVec2(offset.x, offset.y + style::groupbox::height / 2), offset_max, style::themes::active.background_darker, style::general::rounding);

		switch (style::themes::active.tab_style) {
		case style::themes::e_tab_title_style::minimal:
			ImGui::add_rect_filled_half_rounded(drawlist, offset, ImVec2(offset_max.x, offset.y + style::groupbox::height), style::themes::active.frame_fill, 4);
			drawlist->AddRectFilledMultiColor(ImVec2(offset.x, offset.y + (style::groupbox::height)-2), ImVec2(offset_max.x, offset.y + (style::groupbox::height)), a, b, b, a); // ctodo: make a func of this
			break;
		case style::themes::e_tab_title_style::full:
			effects::gradient(drawlist, offset, ImVec2(offset_max.x, offset.y + style::groupbox::height), a, b, (float)style::general::rounding, 0.f, 0.15f); // slowly slides back and forth

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
		
		ImFont* font_default = resources::fonts::map["default"];
		ImFont* font_default_smaller = resources::fonts::map["default_smaller"];

		const float text_x = offset.x + style::frame::size_padding.x;
		const float center_y = offset.y + style::groupbox::height / 2;
		const float title_height = render::text::calc::font("a", font_default).y;

		if (description && *description) { // ctodo: smaller font for desc
			render::text::font(drawlist, title, ImVec2(text_x, center_y - title_height), text_color, font_default); // title above the center line, description below it
			render::text::font(drawlist, description, ImVec2(text_x, center_y), text_color, font_default_smaller);
		}
		else
			render::text::font(drawlist, title, ImVec2(text_x, center_y - title_height / 2), text_color, font_default);
	}

	void begin(const char* title, const char* description, e_group_layout layout) {
		ImDrawList* drawlist_window = ImGui::GetWindowDrawList();
		ImVec2 offset, offset_max, offset_size;
		return_two(&offset, &offset_max, &offset_size);

		body(drawlist_window, offset, offset_max, title, description, layout);
		ImGui::SetCursorPos(ImVec2(offset.x + style::frame::size_padding.x, offset.y + style::frame::size_padding.y + style::groupbox::height));
		ImGui::BeginChild(title, ImVec2(offset_size.x - style::frame::size_padding.x, offset_size.y - style::groupbox::height - (2 * style::frame::size_padding.y)));
	}

	void end() {
		index += 1;
		ImGui::EndChild();
	}
}