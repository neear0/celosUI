/* CTODO: Use ImGui inputsystem, as those are supported across multiple platforms that do not have GetASyncKeyState */

#include "ui.h"

namespace celosia::inputsystem {
	static bool pressed(BYTE key) { // watched keys use the state from refresh(), others are polled
		return registry::watched[key] ? registry::state[key] : key::held(key);
	}

	bool key::down(const DWORD& key) {
		const BYTE k = key & 0xFF;
		const bool is_pressed = pressed(k);

		if (is_pressed && !registry::down[k]) { // key has just went down
			registry::down[k] = true;
			return true;
		}
		if (!is_pressed && registry::down[k]) // reset
			registry::down[k] = false;
		return false;
	}

	bool key::up(const DWORD& key) {
		const BYTE k = key & 0xFF;
		const bool is_pressed = pressed(k);

		if (is_pressed && !registry::up[k]) {
			registry::up[k] = true;
			return false;
		}
		if (!is_pressed && registry::up[k]) {
			registry::up[k] = false;
			return true;
		}
		return registry::watched[k]; // CTODO: watched keys report up whenever they aren't pressed, unwatched keys only on release
	}

	bool key::held(const DWORD& key) { // key is currently down
		return GetAsyncKeyState(key) & 0x8000;
	}

	void key::watch(const DWORD& key) { // add key to the watchlist
		const BYTE k = key & 0xFF;
		if (!registry::watched[k]) {
			registry::watched[k] = true;
			registry::state[k] = false;
		}
	}

	void key::unwatch(const DWORD& key) { // remove key from the watchlist
		registry::watched[key & 0xFF] = false;
	}

	void refresh() { // loop through all of the keys in watchlist and validate them
		for (int k = 0; k < 256; k++)
			if (registry::watched[k])
				registry::state[k] = key::held(k);
	}
}
