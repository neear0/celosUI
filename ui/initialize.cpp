#include "ui.h"

namespace celosia::resources { //ctodo: move to resources/ & make a fontstruct
    bool fonts::add(std::string resourcename, const unsigned char* data, int data_size, int fontsize) {
        // fonts only come from memory compiled into the binary, the font parser isn't safe to run on files someone else could have placed
        ImFontConfig config;
        config.FontDataOwnedByAtlas = false; // static data, the atlas only reads it and must not free it
        ImFont* font = render::io->Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(data), data_size, (float)fontsize, &config);
        if (!font)
            return false;
        fonts::map[resourcename] = font;

        return true;
    }
}

namespace celosia::initialize {
    void context() {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        render::io = &ImGui::GetIO();
        render::io->IniFilename = nullptr; // no imgui.ini / imgui_log.txt
        render::io->LogFilename = nullptr;

        style::themes::initialize();
        style::themes::set(style::themes::dark);

        inputsystem::key::watch(keys::mouse_left);

        if (!platform::blur_supported()) // nothing to blur the desktop with, use the shader background
            style::window::background = style::window::shader;
    }

    void fonts() {
        using namespace resources::font_data;
        resources::fonts::add("default", poppins_regular, poppins_regular_size, 16);
        resources::fonts::add("default_smaller", poppins_regular, poppins_regular_size, 14);

        resources::fonts::add("title", poppins_bold, poppins_bold_size, 24);
    }

    // ctodo: freetype
    // ctodo: image loading (dx11)
}

namespace celosia::ui {
    void begin() {
        effects::new_frame();
        inputsystem::refresh();

        platform::new_frame();
        ImGui::NewFrame();
    }

    void end() {
        ImGui::Render();
        platform::present();
    }
}