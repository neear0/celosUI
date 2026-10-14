#pragma once

namespace celosia::render {
	inline ImGuiIO* io = nullptr;
	inline ImDrawList* drawlist_foreground = nullptr;
	inline ImDrawList* drawlist_background = nullptr;

	struct t_tab_content { // what the open tab drew this frame, the tab switch fades it out and in
		ImDrawList* drawlist = nullptr; // the window's, from vtx_begin to vtx_end: the groupbox panels and titles
		int vtx_begin = 0, vtx_end = 0;
		ImVector<ImDrawList*> children; // the groupboxes' child windows, all of them
	};
	inline t_tab_content tab_content; // cleared every frame, filled by main_window() and groupbox::begin()

	void main_window(); // the demo window, see main_window.cpp
	void titlebar();
	void sidebar();
	void panel(ImDrawList* drawlist, ImVec2 min, ImVec2 max, ImColor color, float rounding); // glass with the shader background, a flat fill otherwise

	namespace text {
		namespace calc {
			ImVec2 font(const char* text, ImFont* font);
		}
		void font(ImDrawList* drawlist, const char* text, ImVec2 pos, ImColor clr, ImFont* font);
		void shadow(const char* text, ImFont* font);
	}

	enum class e_tabs_layout { vertical, horizontal };
	// tab buttons that place and size themselves in the space left in the window: one under the other at full width, or side by side
	// sharing the width. returns the index of the active tab, the first one starts active when none is
	int tabs(std::initializer_list<const char*> labels, e_tabs_layout layout = e_tabs_layout::vertical);

	namespace groupbox {
		enum e_group_layout { fullscreen, two, four };
		inline int index = 0; // position of the next groupbox, reset every frame
		float title_height(const char* description = ""); // of the title at the top, grows when the text needs it
		void body(ImDrawList* drawlist, const ImVec2 offset, const ImVec2 offset_max, const char* title = "", const char* description = "");
		// layout: fullscreen fills the space right of the sidebar, two stack in it, four make a 2x2 grid. groupbox::index picks the cell
		void begin(const char* title, const char* description = "", e_group_layout layout = e_group_layout::two);
		void end();
	}
}