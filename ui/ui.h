// CTODO: check this out & changes here take a while to compile as a result of it being included everywhere

#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <array>
#include <cmath>
#include <string>
#include <string_view>
#include <unordered_map>
#include <map>
#include <filesystem>

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"

#include "resources/resources.h"
#include "style/style.h"
#include "render/render.h"
#include "debug/debug.h"
#include "wrappers/wrappers.h"

namespace celosia {
	namespace initialize {
		void context();
		void fonts();
	}

	namespace resources {
		namespace fonts {
			bool add(std::string resourcename, std::string fontname, int fontsize);
			inline std::unordered_map<std::string, ImFont*> map;
		}
	}

	namespace variables {
		inline static const char* title = "celos";
		inline static const char* version = "UI"; // ctodo: turn into a struct / class
		
		namespace config {
			inline std::unordered_map<std::string, int> ints;
			inline std::unordered_map<std::string, float> floats;
			inline std::unordered_map<std::string, bool> bools;
			inline std::unordered_map<std::string, ImVec2> vec2;
			inline std::unordered_map<std::string, std::string> strings;
		}

		namespace temporary {
			inline std::unordered_map<std::string, int> ints;
			inline std::unordered_map<std::string, float> floats;
			inline std::unordered_map<std::string, bool> bools;
			inline std::unordered_map<std::string, ImVec2> vec2;
			inline std::unordered_map<std::string, std::string> strings;
		}
	}

	namespace monitor {
		inline ImVec2 size;

		void update();
	}

	namespace animations {
		inline std::unordered_map<ImGuiID, ImVec4> values; // one entry per animated value, floats use x and ImVec2 uses x, y
		inline float base_speed = 2.f;

		enum e_method{ smooth, linear };

		namespace functions {
			float calc_linear(float current_value, const float goal_value, float speed);
			float calc_smooth(float current_value, const float goal_value, float speed);

			void tab_switch();
		}

		ImGuiID key(std::string_view name, ImGuiID seed = 0); // chain keys without building strings: key("suffix", key(label))
		float get(ImGuiID id);
		float get(std::string_view name);

		float set(ImGuiID id, const float goal_value, float speed = 0, e_method method = e_method::smooth);
		ImVec2 set(ImGuiID id, const ImVec2 goal_value, float speed = 0, e_method method = e_method::smooth);
		ImVec4 set(ImGuiID id, const ImVec4 goal_value, float speed = 0, e_method method = e_method::smooth);

		float set(std::string_view name, const float goal_value, float speed = 0, e_method method = e_method::smooth);
		ImVec2 set(std::string_view name, const ImVec2 goal_value, float speed = 0, e_method method = e_method::smooth);
		ImVec4 set(std::string_view name, const ImVec4 goal_value, float speed = 0, e_method method = e_method::smooth);
	}

	namespace ui {
		inline ImVec2 size = { 800, 600 };
		inline bool active = true;

		inline std::string tab_active;  // last tab button pressed
		inline std::string tab_current; // tab being drawn, follows tab_active once the switch animation hides the old one

		void begin();
		void end();
		void render();
	}

	namespace functions {
		ImVec2 mouse_vec2();
		bool hovered(const ImVec2& pos1, const ImVec2& pos2);
		
		void spacing(int amount);

		int clamp(int val, const int& min, const int& max);
		float clamp(float val, const float& min, const float& max);
		void clamp(int* val, const int& min, const int& max);
		void clamp(float* val, const float& min, const float& max);
	}

	namespace inputsystem {
		namespace registry { // indexed by virtual key code
			inline std::array<bool, 256> down{};
			inline std::array<bool, 256> up{};
			inline std::array<bool, 256> watched{}; // keys that are actively being refreshed globally
			inline std::array<bool, 256> state{};   // held state of watched keys, updated by refresh()
		}

		namespace key {
			bool down(int key);
			bool up(int key);
			bool held(int key);

			void watch(int key);   // will add to an array that is constantly being checked, unlike others that depend on functions to watch for them
											// this is only efficient if you are trying to monitor the same key in multiple places and don't want to call getasynckeystate each time, or just for easier management of it
											// it is also important to use for first calls on key::down as there's an issue where it detects a key being held down as just being pressed, which it wasn't
											// this is due to the lack of data from not being watched
			void unwatch(int key);
		}

		void refresh();		// this has to be ran every frame
	}

	namespace keys { // key codes used by inputsystem, these match the windows virtual key codes and other platforms translate them
		enum : int {
			mouse_left = 0x01, mouse_right = 0x02, mouse_middle = 0x04,
			backspace = 0x08, tab = 0x09, enter = 0x0D, shift = 0x10, control = 0x11, alt = 0x12, escape = 0x1B, space = 0x20,
			left = 0x25, up = 0x26, right = 0x27, down = 0x28, insert = 0x2D, del = 0x2E, home = 0x24, end = 0x23,
			// '0'-'9' and 'A'-'Z' are their ascii values
			f1 = 0x70, // f1 + n for f(n + 1), up to f12
		};
	}

	namespace platform { // one implementation per backend: win32/ (Win32 + D3D11) and glfw/ (GLFW + Vulkan)
		bool create(const char* title, ImVec2 pos, ImVec2 size); // window, renderer and ImGui backends, call after initialize::context()
		bool poll();         // handles window events, false once the window has been closed
		void new_frame();
		void present();      // renders ImGui's draw data and shows it
		void destroy();

		bool key_held(int key);  // celosia::keys code, works while the window isn't focused where the platform allows it
		ImVec2 cursor_pos();     // screen coordinates
		ImRect window_rect();    // screen coordinates
		ImVec2 screen_size();
		void move(ImVec2 pos);
		void bring_to_top();
	}

	namespace window {
		void drag(); // moves the window while the titlebar is held
	}
}