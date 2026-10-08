// precompiled header, force-included into every project source file (ImGui's own sources don't use it)
// only add headers that rarely change here, editing this file rebuilds everything

#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>

#include <iostream>
#include <array>
#include <cmath>
#include <string>
#include <string_view>
#include <unordered_map>
#include <map>

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include "../external/imgui/dx11/imgui_impl_dx11.h"
#include "../external/imgui/win32/imgui_impl_win32.h"
