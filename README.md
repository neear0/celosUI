# CelosUI
A heavily modified version of ImGui with added functionality.

## Current features
* All of the ImGui features
* Animations
* New and improved widgets and components
* Map variables
* Automatic layouts
* Win32 windows and effects
* Custom theme support
* DirectX 11
* New font system
* Tabs
* InputSystem

## To implement
* Freetype
* Image Loading
* Button Layout (Tabs)
* Theme Lerp & PushStyleColor / Var
* Color Picker
* Scrollbar.h
* Linux support
* Other rendering backends for compatibility
* Shader support
* Improved font system

## Optional
* redo font spacing code
* redo imgui::endchild pushclip, combo pushclip, begin pushbclip
* RenderFrameFunctions
* for animations using renderframeanimations, add an index that is reset on every frame to avoid mixing up

## Building
**Windows (Visual Studio):** open `cUI.sln` and build. This uses Win32 + DirectX 11.

**CMake (Windows and Linux):**
```
cmake -S . -B build
cmake --build build --config Release
```
CMake picks the backend with `CELOSUI_PLATFORM`:
* `win32`: Win32 + DirectX 11, the default on Windows. It's the only backend with the blurred window background.
* `glfw`: GLFW + Vulkan, the default on Linux (it also builds on Windows). GLFW, the Vulkan headers and [volk](https://github.com/zeux/volk) are downloaded while configuring, so Vulkan only has to be installed as a driver.

On Linux you need a compiler, CMake, the X11 development packages and a Vulkan driver. On Debian/Ubuntu:
```
sudo apt install build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev mesa-vulkan-drivers
```
The window runs through X11 (XWayland on Wayland desktops), because Wayland doesn't let windows move themselves or stay on top. `-DCELOSUI_WAYLAND=ON` additionally builds GLFW's native Wayland support, which also needs `libwayland-dev libxkbcommon-dev wayland-protocols`.

The fonts in `assets/` are compiled into the executable through `ui/resources/font_data.cpp`, so nothing is loaded from disk. After changing a font, regenerate that file with `cmake -P ui/resources/embed_fonts.cmake`.

## Known issues
* Adding rounding to a Win32 window that has blur will result in weird looking edges (solution is to use an actual blur shader, which I have not added support for yet)
* Selectable objects will not update colors as a result of the currently broken theme switcher. I've decided to ignore it for now as it will get overwritten regardless.
* Keydown events will overwrite each other if watching the same key

![screenshot](assets/preview.png)

To find todo's, search all files for "CTODO". This was in order to prevent ImGui's todos from interfering with my own.
