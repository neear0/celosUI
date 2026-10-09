#include "ui.h"

namespace celosia::monitor {
	void update(){
		size = platform::screen_size();
	}
}

namespace celosia::functions {
	ImVec2 mouse_vec2() {
		return platform::cursor_pos();
	}

	bool hovered(const ImVec2& pos1, const ImVec2& pos2) {
		ImVec2 mouse_pos = ImGui::GetMousePos();
		return ((mouse_pos.x >= pos1.x && mouse_pos.x <= pos2.x) && (mouse_pos.y >= pos1.y && mouse_pos.y <= pos2.y));
	}

	void spacing(int amount) {
		for (int i = 0; i < amount; i++)
			ImGui::Spacing();
	}

	int clamp(int val, const int& min, const int& max) {
		if (val > max) val = max;
		if (val < min) val = min;
		return val;
	}

	float clamp(float val, const float& min, const float& max) {
		if (val > max) val = max;
		if (val < min) val = min;
		return val;
	}

	void clamp(int* val, const int& min, const int& max) {
		if (*val > max) *val = max;
		if (*val < min) *val = min;
	}

	void clamp(float* val, const float& min, const float& max) {
		if (*val > max) *val = max;
		if (*val < min) *val = min;
	}
}