#include "../ui.h"

void celosia::animations::functions::tab_switch() {
	static std::string tab_old;
	static float progress = 0.f; // 0 shows the tab, 1 has it covered and moved down all the way
	static bool fading_out = false;
	static const ImGuiID spacing = animations::key("groupbox_active_spacing");

	if (tab_old.empty()) // the first tab is shown at once
		tab_old = ui::tab_active;

	if (tab_old != ui::tab_active) {
		fading_out = true; // from wherever it is, so picking another tab mid-switch doesn't jump
		tab_old = ui::tab_active;

		debug::log("tab switch animation activated");
	}

	if (fading_out) {
		progress = ImMin(progress + delta_time / style::frame::tab_switch_out, 1.f);
		if (progress >= 1.f) {
			fading_out = false;
			ui::tab_current = ui::tab_active; // the new groupboxes appear while covered
		}
	}
	else if (progress > 0.f)
		progress = ImMax(progress - delta_time / style::frame::tab_switch_in, 0.f);

	// eased so it starts and stops gently instead of at full speed
	const float amount = progress * progress * (3.f - 2.f * progress);
	values[spacing].x = amount * style::frame::tab_switch_animation_max;

	if (amount > 0.f) {
		ImColor overlay_color = style::themes::active.background;
		overlay_color.Value.w = amount;

		render::drawlist_foreground->AddRectFilled(
			ImVec2(style::sidebar::width, style::titlebar::height),
			ImVec2(style::sidebar::width + ui::size.x - style::frame::size_padding.x, style::titlebar::height + ui::size.y - style::frame::size_padding.y),
			overlay_color
		);
	}
}
