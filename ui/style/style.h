#pragma once

namespace celosia::style {
	// how big the ui is drawn, 1 at 96 dpi. platform::create() sets it from the monitor the window opens on and set_scale()
	// scales the sizes in here along, they're in pixels at scale 1 until then
	inline float scale = 1.f;

	namespace general {
		inline ImColor main_color = { 189, 122, 255,255 };
		inline int rounding = 4;
		inline int rounding_window = 0;
		inline float scroll_speed = 15.f;
		inline float scroll_multiplier = 2.f;
		inline ImVec2 padding = {10, 10};
		inline ImVec2 spacing = {0, 4};
	}
	namespace window {
		enum e_background { blur, shader };
		inline e_background background = blur; // blur: the os blurs the desktop behind the window (windows, kwin on x11), shader: effects::background with glass panels
		inline float noise = 0.04f; // film grain (effects::noise) over the background and panels, how strong it is, 0 turns it off
		inline float glass_blur = 12.f; // radius in pixels of the frosted glass (effects::blur): the popups, and the panels over the shader background
		inline float glass_opacity = 0.4f; // with the blur background: how much the window's background and panels cover the blurred desktop, 0 leaves it bare
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
		inline float border_size = 2.f;

		inline float tab_switch_animation_max = 16; // how far the groupboxes move down while the tab switches
		inline float tab_switch_out = 0.12f; // seconds the old tab takes to fade out
		inline float tab_switch_in = 0.3f; // seconds the new tab takes to fade in
	}
	namespace groupbox {
		inline float height = 40; // of the title, the least it can be, see render::groupbox::title_height()
		inline int padding = titlebar::padding * 4;
		enum e_title_style { full, minimal }; // full: a gradient bar, minimal: a plain bar with a gradient underline
		inline e_title_style title_style = full; // not part of a theme, switching the theme keeps it
	}

	namespace themes {
		struct t_theme {
			std::string name;

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

		ImColor rgb_accent(ImColor clr);
		ImColor rgb_intensity(ImColor clr, float percentage);
	}

	void apply(); // writes the active theme and the sizes above into ImGui's style
	void set_scale(float new_scale); // scales the sizes above, ui::size and ImGui's style sizes from the current scale to new_scale
}