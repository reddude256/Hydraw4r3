#include "custom_elements.h"
#include "render.hpp"

bool c_gui::tab(const char* name, const char* icon, bool active, ImVec2 size_arg) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(name);
    const ImVec2 label_size = CalcTextSize(name, NULL, true);
    const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true);
    DWORD flags = ImGuiWindowFlags_None;


    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
    if (pressed)
        MarkItemEdited(id);

    if (active) {
        window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(37, 37, 51, 255));
        window->DrawList->AddRectFilled(bb.Min, bb.Max - ImVec2(142, 0), GetColorU32(ImGuiCol_SliderGrabActive));
    }


    ImGui::PushFont(iconfont);
    ImGui::RenderText(ImVec2(bb.Min.x + 18 - icon_size.x / 2, bb.Min.y + (size.y / 2) - (icon_size.y / 2)), icon);
    ImGui::PopFont();

    window->DrawList->AddText(bb.Min + ImVec2(30, 7), ImColor(0, 0, 0, 255), name);
    if (hovered)
        window->DrawList->AddText(bb.Min + ImVec2(28, 6), active ? GetColorU32(ImGuiCol_Text) : ImColor(150, 150, 161, 255), name);
    else
        window->DrawList->AddText(bb.Min + ImVec2(28, 6), active ? GetColorU32(ImGuiCol_Text) : ImColor(110, 110, 124, 255), name);

    return pressed;
}
