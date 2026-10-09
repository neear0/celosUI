#include "ui/ui.h"

int main() {
#ifdef _WIN32
    // dlls loaded at runtime only come from system32, never from next to the executable or the working directory
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);
#endif

    celosia::initialize::context();
    celosia::initialize::fonts();

    if (!celosia::platform::create("Celosia UI", ImVec2(0, 0), celosia::ui::size))
        return 1;

    while (celosia::platform::poll())
    {
        celosia::ui::render();
        if(celosia::ui::active)
            celosia::window::drag();

        celosia::platform::bring_to_top();
    }

    // unload everything
    celosia::platform::destroy();
    ImGui::DestroyContext();
	return 0;
}