#pragma once

namespace celosia::style {
	namespace general {
		inline ImColor main_color = { 189, 122, 255,255 };
		inline int rounding = 4;
		inline int rounding_window = 0;
		inline int scroll_speed = 15;
		inline int scroll_multiplier = 2;
		inline ImVec2 padding = {10, 10};
		inline ImVec2 spacing = {0, 4};
	}
	namespace window {
		enum e_background { blur, shader };
		inline e_background background = blur; // blur: the os blurs the desktop behind the window (windows only), shader: effects::background with glass panels
		inline float noise = 0.04f; // film grain (effects::noise) over the background and panels, how strong it is, 0 turns it off
	}
	namespace titlebar {
		inline float height = 45;
		inline int padding = 2;
	}
	namespace sidebar {
		inline float width = 165;
	}
	namespace frame {
		inline ImVec2 size_padding = {10,10};
		inline int border_size = 2;
		inline int border_size_animated = 2;

		inline float tab_switch_animation_max = 16; // how far the groupboxes move down while the tab switches
		inline float tab_switch_out = 0.12f; // seconds the old tab takes to fade out
		inline float tab_switch_in = 0.3f; // seconds the new tab takes to fade in
	}
	namespace groupbox {
		inline float height = 40; // of the title, the least it can be, see render::groupbox::title_height()
		inline int padding = titlebar::padding * 4;
	}
/*
	namespace checkbox {
		inline int size = 10;
	}
*/

	namespace themes {
		enum e_tab_title_style { full, minimal };

		struct t_theme {
			std::string name;
			e_tab_title_style tab_style;

			ImColor background;
			ImColor background_darker;

			ImColor frame_border;
			ImColor frame_fill;

			ImColor idle;
			ImColor held;
			ImColor hovered;
			ImColor active;

			ImColor text;
		};

		inline t_theme dark;
		inline t_theme light;

		inline t_theme active;

		void initialize();
		inline float lerp_speed = 8.f; // how fast set() eases into a new theme
		void set(const t_theme& theme, bool instant = false); // eases active into theme, or switches at once
		void update(); // eases active a step towards the theme given to set() and applies it, ui::begin() calls it every frame

		ImVec4 rgb_vec4(const float& r, const float& g, const float& b, const float& a);
		ImVec4 rgb_vec4(const ImColor& rgba);

		ImColor rgb_accent(ImColor clr);
		ImColor rgb_intensity(ImColor clr, float percentage);
	}

	void apply(); // writes the active theme and the sizes above into ImGui's style
}