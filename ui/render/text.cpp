#include "../ui.h"

namespace celosia::render::text {
	namespace calc {
		ImVec2 font(const char* text, ImFont* font) { // same result as PushFont + CalcTextSize + PopFont, without touching the font stack
			if (!font)
				font = ImGui::GetDefaultFont();

			ImVec2 size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, text);
			size.x = IM_TRUNC(size.x + 0.99999f);
			return size;
		}
	}
	void font(ImDrawList* drawlist, const char* text, ImVec2 pos, ImColor clr, ImFont* font) { // usage: single use text calls that require a different font, to avoid making the code a mess
		if (!font)
			font = ImGui::GetDefaultFont();

		drawlist->AddText(font, font->FontSize, pos, clr, text);
	}
}
