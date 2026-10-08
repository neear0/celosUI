#include "../ui.h"

namespace celosia::animations::functions {
	void tab_switch() {
		static std::string tab_old;
		static bool animation_active = false;
		static bool animation_down = false;
		static const ImGuiID spacing = animations::key("groupbox_active_spacing");

		if (tab_old != ui::tab_active) {
			animation_active = true;
			animation_down = true;

			tab_old = ui::tab_active;

			debug::log("tab switch animation activated");
		}

		if (animation_active) {
			if (animation_down)
				animations::set(spacing, style::frame::tab_switch_animation_max * 2, style::frame::tab_switch_animation_speed, animations::e_method::smooth);
			else
				animations::set(spacing, 0.f, style::frame::tab_switch_animation_speed, animations::e_method::smooth);

			if (animations::get(spacing) >= style::frame::tab_switch_animation_max)
				animation_down = false;

			if (!animation_down && animations::get(spacing) == 0)
				animation_active = false;

			float overlay_alpha = animations::get(spacing) / style::frame::tab_switch_animation_max;
			ImColor overlay_color = style::themes::active.background;
			overlay_color.Value.w = overlay_alpha;

			render::drawlist_foreground->AddRectFilled(
				ImVec2(style::sidebar::width, style::titlebar::height),
				ImVec2(style::sidebar::width + ui::size.x - style::frame::size_padding.x, style::titlebar::height + ui::size.y - style::frame::size_padding.y),
				overlay_color
			);

			if (!animation_down)
				ui::tab_current = ui::tab_active; // allow the new groupbox to become visible
		}
	}
}
