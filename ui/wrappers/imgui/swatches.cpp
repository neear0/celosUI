#include "../../ui.h"

// swatches(): a row of round color choices, as tall as a frame, with the label after it. the one col is set to has a ring
// in the text color, hovered ones glow in their own color like color_edit()'s swatch

static bool same_color(const float col[4], ImColor color) { // to an 8 bit step
    const ImVec4 c = color.Value;
    return fabsf(col[0] - c.x) < 0.5f / 255.f && fabsf(col[1] - c.y) < 0.5f / 255.f && fabsf(col[2] - c.z) < 0.5f / 255.f;
}

bool celosia::widgets::swatches(const char* label, float col[4], const ImColor* colors, int count) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems || count <= 0)
        return false;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
    const float size = ImGui::GetFrameHeight();
    const float gap = style.ItemInnerSpacing.x;
    const ImVec2 pos = window->DC.CursorPos;
    const float row = size * count + gap * (count - 1);
    const ImRect total(pos, pos + ImVec2(row + (label_size.x > 0.f ? gap + label_size.x : 0.f), size));
    ImGui::ItemSize(total, style.FramePadding.y);
    if (!ImGui::ItemAdd(total, id))
        return false;

    ImDrawList* drawlist = window->DrawList;
    const ImVec4 text = celosia::style::themes::active.text.Value;
    const float scale = celosia::style::scale;
    bool changed = false;
    for (int i = 0; i < count; i++) {
        const ImRect rect(ImVec2(pos.x + (size + gap) * i, pos.y), ImVec2(pos.x + (size + gap) * i + size, pos.y + size));
        const ImGuiID item = ImHashData(&i, sizeof(i), id); // from the row's id, two rows in one window don't share their swatches
        if (!ImGui::ItemAdd(rect, item))
            continue;
        bool hovered, held;
        if (ImGui::ButtonBehavior(rect, item, &hovered, &held) && !same_color(col, colors[i])) {
            col[0] = colors[i].Value.x;
            col[1] = colors[i].Value.y;
            col[2] = colors[i].Value.z;
            col[3] = 1.f;
            changed = true;
            ImGui::MarkItemEdited(id);
        }

        const ImVec4 color = colors[i].Value;
        const ImVec2 center = rect.GetCenter();
        const float picked = celosia::animations::set(celosia::animations::key("swatch_picked", item), same_color(col, colors[i]) ? 1.f : 0.f, celosia::animations::color_speed);
        const float glow = celosia::animations::set(celosia::animations::key("swatch_glow", item), hovered ? 1.f : 0.f);
        const float radius = size * 0.5f - 4.f * scale * picked; // shrinks inside the ring once it's picked
        if (glow > 0.01f)
            celosia::effects::glow(drawlist, center - ImVec2(radius, radius), center + ImVec2(radius, radius), ImColor(color.x, color.y, color.z, 0.5f * glow), radius, 8.f * scale);
        drawlist->AddCircleFilled(center, radius, ImColor(color.x, color.y, color.z, 1.f));
        if (picked > 0.01f)
            drawlist->AddCircle(center, size * 0.5f - 1.f * scale, ImColor(text.x, text.y, text.z, 0.9f * picked), 0, 2.f * scale);
    }
    if (label_size.x > 0.f)
        ImGui::RenderText(ImVec2(pos.x + row + gap, pos.y + (size - label_size.y) * 0.5f), label);

    return changed;
}
