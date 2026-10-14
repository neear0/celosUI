#include "ui.h"

void celosia::ui::render() {
    begin();

    render::drawlist_background = ImGui::GetBackgroundDrawList();
    render::drawlist_foreground = ImGui::GetForegroundDrawList();
    render::tab_content.drawlist = nullptr;
    render::tab_content.children.resize(0);

    if (ui::active) {
        render::groupbox::index = 0;
        render::main_window();
        window::drag();
    }

    /* use drawlists here */

    animations::functions::tab_switch();
    end();
}
