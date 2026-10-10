#pragma once

// extra ImGui components, kept in one place. ImGui widgets that are replaced outright (ButtonEx, Checkbox, Combo, Scrollbar,
// the color editor) keep their ImGui declarations and live in ImGui/ next to these
namespace ImGui {
	// widgets
	bool tab_button(const char* label);
	bool tab_button_ex(const char* label, ImGuiButtonFlags flags);
	bool checkbox_map(const char* label); // Checkbox() on variables::config::bools[label]

	// drawing
	void add_rect_filled_half_rounded(ImDrawList* drawlist, const ImVec2& min, const ImVec2& max, const ImColor& clr, const int& rounding);

	// item frames, ImGui/functions/
	IMGUI_API void render_frame_border_color(ImVec2 p_min, ImVec2 p_max, ImColor clr,float rounding = 0.0f);
	IMGUI_API void render_frame_border_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, float rounding = 0.0f);
	IMGUI_API void render_frame_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, bool border = true, float rounding = 0.0f);

	// smooth scrolling, ImGui/Scrollbar.cpp
	void scroll_wheel(ImGuiWindow* window, float wheel); // called by ImGui::UpdateMouseWheel, sets where the window scrolls to
}
