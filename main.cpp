#include "ui/ui.h"

int main() {
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
	return 0;
}