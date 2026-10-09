#include "ui.h"

namespace celosia::resources { //ctodo: move to resources/ & make a fontstruct
    bool fonts::add(std::string resourcename, std::string fontname, int fontsize) {
        std::string fontpath = fontname; // "H:\\SRC\\fonts\\" + fontname;
        if (!std::filesystem::exists(fontpath)) // missing font falls back to the default one, AddFontFromFileTTF would assert
            return false;
        fonts::map[resourcename] = render::io->Fonts->AddFontFromFileTTF(fontpath.c_str(), fontsize);

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
    }

    void fonts() { // ctodo: add bytes into a .h file so there's no need for external files
        resources::fonts::add("default", "Poppins-Regular.ttf", 16);
        resources::fonts::add("default_smaller", "Poppins-Regular.ttf", 14);

        resources::fonts::add("title", "Poppins-Bold.ttf", 24);
    } 

    // ctodo: freetype
    // ctodo: image loading (dx11)
}

namespace celosia::ui {
    void begin() {
        inputsystem::refresh();

        platform::new_frame();
        ImGui::NewFrame();
    }

    void end() {
        ImGui::Render();
        platform::present();
    }
}