#include "../ui.h"

ImColor celosia::style::themes::rgb_accent(ImColor clr) { // a neighbouring hue, a little more saturated, for gradients from the main color
	float h, s, v;
	ImGui::ColorConvertRGBtoHSV(clr.Value.x, clr.Value.y, clr.Value.z, h, s, v);
	h = ImFmod(h + 0.02f, 1.f);
	s = ImMin(s * 1.3f, 1.f);
	return ImColor::HSV(h, s, v, clr.Value.w);
}

ImColor celosia::style::themes::rgb_intensity(ImColor clr, float percentage) {
	return ImColor(clr.Value.x * percentage, clr.Value.y * percentage, clr.Value.z * percentage, clr.Value.w);
}
