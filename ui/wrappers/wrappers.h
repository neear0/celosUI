#pragma once

namespace ImGui { // add extra ImGui components here, in order to keep it all in one place
	bool tab_button(const char* label);
	bool tab_button_ex(const char* label, ImGuiButtonFlags flags);
	void add_rect_filled_multi_color_rounded(ImDrawList* drawlist, const ImVec2 min, const ImVec2 max, const ImColor one, const ImColor two, const int& rounding);
	void add_rect_filled_half_rounded(ImDrawList* drawlist, const ImVec2& min, const ImVec2& max, const ImColor& clr, const int& rounding);

	IMGUI_API void render_frame_border_color(ImVec2 p_min, ImVec2 p_max, ImColor clr,float rounding = 0.0f);
	IMGUI_API void render_frame_border_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, float rounding = 0.0f);
	IMGUI_API void render_frame_animated(const char* label, ImVec2 p_min, ImVec2 p_max, ImColor fill_col, bool border = true, float rounding = 0.0f);

	bool checkbox_map(const char* label);
}

// CTODO: restructure these