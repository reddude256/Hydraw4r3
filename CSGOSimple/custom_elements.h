#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <windows.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace ImGui;

class c_gui
{
public:
    bool tab(const char* name, const char* icon, bool active, ImVec2 size_arg);
};

