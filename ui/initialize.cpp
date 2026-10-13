#include "ui.h"

void celosia::initialize::context() {
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    render::io = &ImGui::GetIO();
    render::io->IniFilename = nullptr; // no imgui.ini / imgui_log.txt
    render::io->LogFilename = nullptr;

    style::themes::initialize();
    style::themes::set(style::themes::dark, true);

    inputsystem::key::watch(keys::mouse_left);
    // platform::create() switches style::window::background to the shader when nothing can blur the desktop
}

void celosia::initialize::fonts() {
    using namespace resources::font_data;
    resources::fonts::add("default", { poppins_regular, poppins_regular_size, 16.f }); // the first one is ImGui's default font
    resources::fonts::add("default_smaller", { poppins_regular, poppins_regular_size, 14.f });
    resources::fonts::add("title", { poppins_bold, poppins_bold_size, 24.f });
}

void celosia::initialize::images() {
    using namespace resources::image_data;
    resources::images::add("test", test, test_size);
}

void celosia::ui::begin() {
    effects::new_frame();
    inputsystem::refresh();
    style::themes::update();

    platform::new_frame();
    ImGui::NewFrame();
    animations::new_frame();
}

void celosia::ui::end() {
    ImGui::Render();
    platform::present();
}
