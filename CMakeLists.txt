cmake_minimum_required(VERSION 3.20)
project(celos_ui LANGUAGES C CXX)

# win32: Win32 window + D3D11, keeps the acrylic blur (windows only)
# glfw:  GLFW window + Vulkan, used on linux and also builds on windows
if(WIN32)
    set(celos_ui_default_platform win32)
else()
    set(celos_ui_default_platform glfw)
endif()
set(CELOSUI_PLATFORM ${celos_ui_default_platform} CACHE STRING "Window and renderer backend: win32 or glfw")
set_property(CACHE CELOSUI_PLATFORM PROPERTY STRINGS win32 glfw)
option(CELOSUI_WAYLAND "Build GLFW with native Wayland support (needs the wayland dev packages), X11 is used when both are available" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # the c runtime is linked statically, so the only dlls left to load are windows' own
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    # control flow guard and cet shadow stack support for release builds, set here so the downloaded dependencies get them too
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/guard:cf>)
    add_link_options($<$<NOT:$<CONFIG:Debug>>:/guard:cf> $<$<NOT:$<CONFIG:Debug>>:/CETCOMPAT>)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # some distributions' compilers add these on their own, others don't, so ask for them explicitly (dependencies included)
    include(CheckCCompilerFlag)
    include(CheckPIESupported)
    check_pie_supported()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    add_compile_options(-fstack-protector-strong)
    foreach(flag -fstack-clash-protection -fcf-protection)
        string(MAKE_C_IDENTIFIER "celos_ui_has${flag}" has_flag)
        check_c_compiler_flag(${flag} ${has_flag})
        if(${has_flag})
            add_compile_options(${flag})
        endif()
    endforeach()
    # fortify needs optimization, so debug builds go without it
    add_compile_options("$<$<CONFIG:Release,RelWithDebInfo,MinSizeRel>:-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=3>")
    add_link_options(-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack)
endif()

set(imgui_sources
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
)

set(celos_ui_sources
    main.cpp
    ui/debug/debug.cpp
    ui/functions.cpp
    ui/initialize.cpp
    ui/inputsystem.cpp
    ui/ui.cpp
    ui/window_drag.cpp
    ui/render/groupbox.cpp
    ui/render/sidebar.cpp
    ui/render/text.cpp
    ui/render/titlebar.cpp
    ui/resources/font_data.cpp
    ui/style/accents.cpp
    ui/style/animations.cpp
    ui/style/tab_switch.cpp
    ui/style/theme_setup.cpp
    ui/wrappers/ImGui/add_rect_filled_half_rounded.cpp
    ui/wrappers/ImGui/add_rect_filled_multi_color_rounded.cpp
    ui/wrappers/ImGui/Button.cpp
    ui/wrappers/ImGui/Checkbox.cpp
    ui/wrappers/ImGui/ColorPicker.cpp
    ui/wrappers/ImGui/Combo.cpp
    ui/wrappers/ImGui/Scrollbar.cpp
    ui/wrappers/ImGui/tab_button.cpp
    ui/wrappers/ImGui/functions/render_frame_animated.cpp
    ui/wrappers/ImGui/functions/render_frame_border_animated.cpp
    ui/wrappers/ImGui/functions/render_frame_border_color.cpp
)

if(CELOSUI_PLATFORM STREQUAL "win32")
    if(NOT WIN32)
        message(FATAL_ERROR "CELOSUI_PLATFORM=win32 only builds on windows, use glfw")
    endif()

    list(APPEND celos_ui_sources
        ui/win32/d3d_create.cpp
        ui/win32/platform_win32.cpp
        ui/win32/window_effects.cpp
        ui/win32/window_functions.cpp
    )
    set(backend_sources
        external/imgui/dx11/imgui_impl_dx11.cpp
        external/imgui/win32/imgui_impl_win32.cpp
    )
elseif(CELOSUI_PLATFORM STREQUAL "glfw")
    include(FetchContent)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND ${CELOSUI_WAYLAND} CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
        URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
        URL_HASH SHA256=c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01
    )
    # headers and volk are only downloaded, their own cmake projects aren't needed (SOURCE_SUBDIR has no CMakeLists.txt)
    FetchContent_Declare(vulkan_headers
        URL https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/vulkan-sdk-1.3.296.0.tar.gz
        URL_HASH SHA256=1e872a0be3890784bbe68dcd89b7e017fed77ba95820841848718c98bda6dc33
        SOURCE_SUBDIR none
    )
    FetchContent_Declare(volk
        URL https://github.com/zeux/volk/archive/refs/tags/vulkan-sdk-1.3.296.0.tar.gz
        URL_HASH SHA256=8ffd0e81e29688f4abaa39e598937160b098228f37503903b10d481d4862ab85
        SOURCE_SUBDIR none
    )
    FetchContent_MakeAvailable(glfw vulkan_headers volk)

    list(APPEND celos_ui_sources
        ui/glfw/platform_glfw.cpp
        ui/glfw/vulkan_create.cpp
    )
    set(backend_sources
        external/imgui/glfw/imgui_impl_glfw.cpp
        external/imgui/vulkan/imgui_impl_vulkan.cpp
        ${volk_SOURCE_DIR}/volk.c
    )
else()
    message(FATAL_ERROR "unknown CELOSUI_PLATFORM '${CELOSUI_PLATFORM}', use win32 or glfw")
endif()

add_executable(celos_ui ${celos_ui_sources} ${imgui_sources} ${backend_sources})

# ui/pch.h is force-included into our sources (the vcxproj does the same), ImGui, the backends and the font data build without it
target_precompile_headers(celos_ui PRIVATE ui/pch.h)
set_source_files_properties(${imgui_sources} ${backend_sources} ui/resources/font_data.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

if(CELOSUI_PLATFORM STREQUAL "win32")
    target_compile_definitions(celos_ui PRIVATE UNICODE _UNICODE) # same character set as the vcxproj
    target_link_libraries(celos_ui PRIVATE d3d11 d3dcompiler dwmapi)
else()
    target_include_directories(celos_ui PRIVATE ${vulkan_headers_SOURCE_DIR}/include ${volk_SOURCE_DIR})
    target_compile_definitions(celos_ui PRIVATE IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
    target_link_libraries(celos_ui PRIVATE glfw ${CMAKE_DL_LIBS})
endif()

if(MSVC)
    target_compile_options(celos_ui PRIVATE /MP)
    # imported dlls are only looked up in system32, never next to the executable
    target_link_options(celos_ui PRIVATE $<$<NOT:$<CONFIG:Debug>>:/DEPENDENTLOADFLAG:0x800>)
endif()
