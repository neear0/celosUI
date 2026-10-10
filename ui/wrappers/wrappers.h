#pragma once

// extra ImGui components, kept in one place. ImGui widgets that are replaced outright (ButtonEx, Checkbox, Combo, Scrollbar,
// the color editor) keep their ImGui declarations and live in ImGui/ next to these
namespace ImGui {
	// widgets
	bool tab_button(const char* label, const ImVec2& size = ImVec2(0, 0)); // render::tabs() lays these out, size 0 is the sidebar's width and the text's height
	bool tab_button_ex(const char* label, const ImVec2& size, ImGuiButtonFlags flags);
	bool checkbox_map(const char* label); // Checkbox() on variables::config::bools[label]
	void image(const char* name, ImVec2 size = ImVec2(0, 0), float rounding = 0.f); // a resources::images image as an item, size 0 is its own size
	bool color_edit(const char* label, float col[4], bool alpha = true);   // swatch that opens a color picker popup, true when the color changed
	bool color_picker(const char* label, float col[4], bool alpha = true); // saturation/value square, hue bar and alpha bar

	// drawing
	void add_rect_filled_half_rounded(ImDrawList* drawlist, const ImVec2& min, const ImVec2& max, const ImColor& clr, const int& rounding);

	// item frames, ImGui/functions/
	IMGUI_API void render_frame_border_color(ImVec2 p_min, ImVec2 p_max, ImColor clr,float rounding = 0.0f);
	IMGUI_API void render_frame_border_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, float rounding = 0.0f);
	IMGUI_API void render_frame_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, bool border = true, float rounding = 0.0f);

	// smooth scrolling, ImGui/Scrollbar.cpp
	void scroll_wheel(ImGuiWindow* window, float wheel); // called by ImGui::UpdateMouseWheel, sets where the window scrolls to
}
