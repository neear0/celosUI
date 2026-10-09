#include "ui.h"

float color[4] = { celosia::style::general::main_color.Value.x, celosia::style::general::main_color.Value.y, celosia::style::general::main_color.Value.z, celosia::style::general::main_color.Value.w};

namespace celosia::ui { // ctodo: move this to render
    void main_window() {
        ImGui::Begin(variables::title, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowSize(celosia::ui::size);
        ImGui::SetWindowPos(ImVec2(0, 0));

        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        const ImVec2 pos = ImGui::GetWindowPos();
        drawlist->PushClipRect(pos, pos + celosia::ui::size); // ImGui clips a window's contents one pixel in from its edges, the background, titlebar and sidebar go all the way
        if (style::window::background == style::window::shader) {
            const ImVec4 main = style::general::main_color.Value;
            const ImVec4 accent = style::themes::rgb_accent(style::general::main_color).Value;
            ImVec4 base = style::themes::active.background.Value;
            base.w = 1.f; // the window is opaque with the shader background, only the rounded corners let the desktop through
            effects::background(drawlist, pos, pos + celosia::ui::size,
                base, ImColor(main.x, main.y, main.z, 0.55f), ImColor(accent.x, accent.y, accent.z, 0.45f), (float)style::general::rounding_window);
        }
        render::titlebar();
        render::sidebar();
        drawlist->PopClipRect();

        if (tab_current == "tab1") { // --> visible tab
            render::groupbox::begin("Groupbox A");
            for (int i = 0; i < 20; i++)
                if (ImGui::Button(("group A " + std::to_string(i)).c_str()))
                    debug::log("Button " + std::to_string(i) + " has been hit");
            render::groupbox::end();

            render::groupbox::begin("Groupbox B", "Checkboxes");
            for (int i = 0; i < 20; i++)
                ImGui::checkbox_map(("Checkbox " + std::to_string(i)).c_str());
            render::groupbox::end();
        }
        else {
            render::groupbox::begin("Groupbox B");
            const char* test[] = {"Item A", "Item B", "Item C"};
            ImGui::Combo("test", &variables::config::ints["test_combo"], test, 3, 3);

            ImGui::ColorEdit4("Main Color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview /*| ImGuiColorEditFlags_AlphaBar*/); // ctodo: remake this
            style::general::main_color = { color[0], color[1], color[2], color[3] };

            render::groupbox::end();
        }

        ImGui::End();
    }

    void render() {
        begin();

        render::drawlist_background = ImGui::GetBackgroundDrawList();
        render::drawlist_foreground = ImGui::GetForegroundDrawList();

        if (ui::active) {
            render::groupbox::index = 0;
            main_window();
        }

        /* use drawlists here */

        animations::functions::tab_switch();
        end();
    }
}