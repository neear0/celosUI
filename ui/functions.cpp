#include "ui.h"

void celosia::monitor::update(){
	size = platform::screen_size();
}

ImVec2 celosia::functions::mouse_vec2() {
	return platform::cursor_pos();
}

bool celosia::functions::hovered(const ImVec2& pos1, const ImVec2& pos2) {
	ImVec2 mouse_pos = ImGui::GetMousePos();
	return ((mouse_pos.x >= pos1.x && mouse_pos.x <= pos2.x) && (mouse_pos.y >= pos1.y && mouse_pos.y <= pos2.y));
}

void celosia::functions::spacing(int amount) {
	for (int i = 0; i < amount; i++)
		ImGui::Spacing();
}

int celosia::functions::clamp(int val, const int& min, const int& max) {
	if (val > max) val = max;
	if (val < min) val = min;
	return val;
}

float celosia::functions::clamp(float val, const float& min, const float& max) {
	if (val > max) val = max;
	if (val < min) val = min;
	return val;
}

void celosia::functions::clamp(int* val, const int& min, const int& max) {
	if (*val > max) *val = max;
	if (*val < min) *val = min;
}

void celosia::functions::clamp(float* val, const float& min, const float& max) {
	if (*val > max) *val = max;
	if (*val < min) *val = min;
}
