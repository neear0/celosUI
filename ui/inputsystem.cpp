#include "ui.h"

namespace celosia::inputsystem {
	static bool pressed(unsigned char key) { // watched keys use the state from refresh(), others are polled
		return registry::watched[key] ? registry::state[key] : key::held(key);
	}

	bool key::down(int key) {
		const unsigned char k = key & 0xFF;
		const bool is_pressed = pressed(k);

		if (is_pressed && !registry::down[k]) { // key has just went down
			registry::down[k] = true;
			return true;
		}
		if (!is_pressed && registry::down[k]) // reset
			registry::down[k] = false;
		return false;
	}

	bool key::up(int key) {
		const unsigned char k = key & 0xFF;
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

	bool key::held(int key) { // key is currently down
		return platform::key_held(key);
	}

	void key::watch(int key) { // add key to the watchlist
		const unsigned char k = key & 0xFF;
		if (!registry::watched[k]) {
			registry::watched[k] = true;
			registry::state[k] = false;
		}
	}

	void key::unwatch(int key) { // remove key from the watchlist
		registry::watched[key & 0xFF] = false;
	}

	void refresh() { // loop through all of the keys in watchlist and validate them
		for (int k = 0; k < 256; k++)
			if (registry::watched[k])
				registry::state[k] = key::held(k);
	}
}
