#pragma once

// our widgets, built on ImGui's item and input handling, in wrappers/imgui/. ImGui widgets that are replaced outright
// (ButtonEx, Checkbox, BeginCombo and Combo, Selectable, Scrollbar) keep their ImGui names and declarations, ImGui itself
// calls them. ImGui's color editor is gone, see color_edit()
namespace celosia::widgets {
	// widgets
	bool tab_button(const char* label, const ImVec2& size = ImVec2(0, 0)); // render::tabs() lays these out, size 0 is the sidebar's width and the text's height
	bool tab_button_ex(const char* label, const ImVec2& size, ImGuiButtonFlags flags);
	bool checkbox_map(const char* label); // ImGui::Checkbox() on variables::config::bools[label]
	void image(const char* name, ImVec2 size = ImVec2(0, 0), float rounding = 0.f); // a resources::images image as an item, size 0 is its own size
	bool color_edit(const char* label, float col[4], bool alpha = true);   // swatch that opens a color picker popup, true when the color changed
	bool color_picker(const char* label, float col[4], bool alpha = true); // saturation/value square, hue bar and alpha bar
	bool slider_float(const char* label, float* v, float min, float max, const char* format = "%.1f"); // dragged along a track, true when the value changed. format shows the value

	// drawing
	void add_rect_filled_half_rounded(ImDrawList* drawlist, const ImVec2& min, const ImVec2& max, const ImColor& clr, float rounding);

	// item frames, imgui/functions/. id is the item's, the animated ones ease their color per item
	void render_frame_border_color(ImVec2 p_min, ImVec2 p_max, ImColor clr, float rounding = 0.0f);
	void render_frame_border_animated(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, float rounding = 0.0f);
	void render_frame_animated(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, float rounding = 0.0f);

	// smooth scrolling, imgui/scrollbar.cpp
	void scroll_wheel(ImGuiWindow* window, float wheel); // called by ImGui::UpdateMouseWheel, sets where the window scrolls to
}
