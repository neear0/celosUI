#include "ui.h"

namespace celosia::initialize {
    void context() {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        render::io = &ImGui::GetIO();
        render::io->IniFilename = nullptr; // no imgui.ini / imgui_log.txt
        render::io->LogFilename = nullptr;

        style::themes::initialize();
        style::themes::set(style::themes::dark, true);

        inputsystem::key::watch(keys::mouse_left);

        if (!platform::blur_supported()) // nothing to blur the desktop with, use the shader background
            style::window::background = style::window::shader;
    }

    void fonts() {
        using namespace resources::font_data;
        resources::fonts::add("default", { poppins_regular, poppins_regular_size, 16.f }); // the first one is ImGui's default font
        resources::fonts::add("default_smaller", { poppins_regular, poppins_regular_size, 14.f });
        resources::fonts::add("title", { poppins_bold, poppins_bold_size, 24.f });
    }

    void images() {
        using namespace resources::image_data;
        resources::images::add("logo", logo, logo_size);
    }
}

namespace celosia::ui {
    void begin() {
        effects::new_frame();
        inputsystem::refresh();
        style::themes::update();

        platform::new_frame();
        ImGui::NewFrame();
    }

    void end() {
        ImGui::Render();
        platform::present();
    }
}