#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "features/item_definitions.h"
#include "features/kit_parser.h"
#include "features/skins.h"
#include "render.hpp"
#include "custom_elements.h"
#include "menu.hpp"


void ReadDirectory(const std::string& name, std::vector<std::string>& v)
{
	auto pattern(name);
	pattern.append("\\*.ini");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			v.emplace_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}
}
struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};

namespace ImGuiEx
{
	inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}
	inline bool ColorEdit4a(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}

	inline bool ColorEdit3(const char* label, Color* v)
	{
		return ColorEdit4(label, v, false);
	}
}

void Menu::Initialize()
{
	CreateStyle();

	_visible = true;
}

void Menu::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

c_gui gui;
void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;
	if (!_visible)
		return;

	static int tab = 0;
	ImGui::Begin("hydraw4r3", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		ImGui::SetWindowSize(ImVec2(870, 600));
		auto p = ImGui::GetWindowPos();
		auto __draw = ImGui::GetWindowDrawList();
		static int tab = 0;

		__draw->AddRectFilled(p + ImVec2(0, 30), p + ImVec2(150, 595), ImColor(31, 31, 41, 255));

		ImGui::SetCursorPos(ImVec2(0, 30));
		ImGui::BeginGroup();
		{
			if (gui.tab("Aimbot", "A", 0 == tab, ImVec2(150, 25)))
				tab = 0;
			if (gui.tab("Visuals", "I", 1 == tab, ImVec2(150, 25)))
				tab = 1;
			if (gui.tab("Misc", "D", 2 == tab, ImVec2(150, 25)))
				tab = 2;
			if (gui.tab("Config", "E", 3 == tab, ImVec2(150, 25)))
				tab = 3;
			if (gui.tab("Skins", "G", 4 == tab, ImVec2(150, 25)))
				tab = 4;
			if (gui.tab("HvH", "I", 5 == tab, ImVec2(150, 25)))
				tab = 5;
		}
		ImGui::EndGroup();

		ImGui::SetCursorPos(ImVec2(156, 30));
		ImGui::BeginGroup();
		{
			switch (tab) {
			case 0:
				ImGui::BeginChild("Aimbot", ImVec2(350, 565));
				{
					static int definition_index = WEAPON_INVALID;

					auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
					if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
						definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
					else
						definition_index = WEAPON_INVALID;
					if (definition_index == WEAPON_INVALID)definition_index = WEAPON_DEAGLE;
					ImGui::BeginGroup();
					{
						auto settings = &g_Options.weapons[definition_index].legit;
						ImGui::Separator("Aimbot");
						ImGui::Checkbox("Enabled", &settings->enabled);

						ImGui::Combo("Silent", &settings->silent2, "Off\0Silent \0Perfect silent\0");
						ImGui::Checkbox("Flash check", &settings->flash_check);
						ImGui::Checkbox("Smoke check", &settings->smoke_check);
						ImGui::Checkbox("Auto-pistol", &settings->autopistol);

						if (ImGui::BeginCombo("##hitbox_filter", "Hitboxes", ImGuiComboFlags_NoArrowButton))
						{
							ImGui::Selectable("Head", &settings->hitboxes.head, ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable("Chest", &settings->hitboxes.chest, ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable("Hands", &settings->hitboxes.hands, ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable("Legs", &settings->hitboxes.legs, ImGuiSelectableFlags_DontClosePopups);

							ImGui::EndCombo();
						}

						ImGui::SliderFloat("Fov", &settings->fov, 0.f, 20.f, "%1.f°");
						if (settings->silent2) {
							ImGui::SliderFloat("Silent fov", &settings->silent_fov, 0.f, 20.f, "%1.f");
						}

						ImGui::SliderFloat("Smooth", &settings->smooth, 1.f, 20.f, "%1.f");

						ImGui::Separator("Delays");

						if (!settings->silent2 || settings->silent2 == 2) {

							ImGui::SliderInt("Shot delay", &settings->shot_delay, 0, 1000, "%i ms");
						}
						ImGui::SliderInt("Kill delay", &settings->kill_delay, 0, 1000, "%i ms");
					}
					ImGui::EndGroup();
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::BeginChild("Recoil Control", ImVec2(360, 250));
					{
						ImGui::BeginGroup();
						{
							ImGui::Separator("Recoil Control ");

							static int definition_index = WEAPON_INVALID;

							auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
							if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
								definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
							else
								definition_index = WEAPON_INVALID;

							if (definition_index == WEAPON_INVALID)
								definition_index = WEAPON_DEAGLE;

							auto settings = &g_Options.weapons[definition_index].legit;
							ImGui::Checkbox("Enabled##rcs", &settings->rcs.enabled);

							const char* rcs_types[] = {
								"Standalone",
								"Aim"
							};


							if (ImGui::BeginCombo("##type", rcs_types[settings->rcs.type], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(rcs_types); i++)
								{
									if (ImGui::Selectable(rcs_types[i], i == settings->rcs.type))
										settings->rcs.type = i;
								}

								ImGui::EndCombo();
							}
							ImGui::SliderInt("X", &settings->rcs.x, 0, 100, "%i");
							ImGui::SliderInt("Y", &settings->rcs.y, 0, 100, "%i");
						}
						ImGui::EndGroup();
					}
					ImGui::EndChild();

					ImGui::BeginChild("Misc##Aimbot", ImVec2(360, 310));
					{
						ImGui::BeginGroup();
						{
							ImGui::Separator("Exploits");
							static int definition_index = WEAPON_INVALID;

							auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
							if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
								definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
							else
								definition_index = WEAPON_INVALID;
							float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

							auto settings = &g_Options.weapons[definition_index].legit;
							ImGui::SliderInt("Backtrack ticks", &settings->backtrack.ticks, 1, 12, "%i");
							ImGui::Checkbox("Enabled##autofire", &settings->autofire.enabled);
							ImGui::SameLine(group_w - 50);
							ImGui::Hotkey("##autofire", &settings->autofire.hotkey);
						}
						ImGui::EndGroup();
					}
					ImGui::EndChild();
				}
				ImGui::EndGroup();
				break;
			case 1:
				ImGui::BeginChild("Visuals", ImVec2(350, 565));
				{
					ImGui::BeginGroup();
					{
						ImGui::Separator("ESP");

						//	ImGui::Checkbox("Enabled",&g_Options.esp_enabled);
							//ImGui::Checkbox("Team check",&g_Options.esp_enemies_only);
						ImGui::Checkbox("Boxes", &g_Options.esp_player_boxes); ImGui::SameLine(0, -20); ImGuiEx::ColorEdit4("Enemies Visible   ", &g_Options.color_esp_enemy_visible);
						ImGui::Checkbox("Occluded ", &g_Options.esp_player_boxesOccluded); ImGui::SameLine(0, -20); ImGuiEx::ColorEdit4("Enemies Occluded      ", &g_Options.color_esp_enemy_occluded);

						ImGui::Checkbox("Names", &g_Options.esp_player_names);
						ImGui::Checkbox("Health", &g_Options.esp_player_health);
						//ImGui::Checkbox("Armour",&g_Options.esp_player_armour);
						ImGui::Checkbox("Weapon", &g_Options.esp_player_weapons);
						//ImGui::Checkbox("Snaplines",&g_Options.esp_player_snaplines);
						ImGui::Checkbox("Dropped Weapons", &g_Options.esp_dropped_weapons);
						//ImGuiEx::ColorEdit4("Allies Visible",&g_Options.color_esp_ally_visible);
						//ImGuiEx::ColorEdit4("Enemies Visible",&g_Options.color_esp_enemy_visible);
						//ImGuiEx::ColorEdit4("Allies Occluded",&g_Options.color_esp_ally_occluded);
						//ImGuiEx::ColorEdit4("Enemies Occluded",&g_Options.color_esp_enemy_occluded);
						//ImGuiEx::ColorEdit4("Dropped Weapons",&g_Options.color_esp_weapons);
					}
					ImGui::EndGroup();
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::BeginChild("Chams", ImVec2(360, 250));
					{
						ImGui::BeginGroup();
						{
							ImGui::Separator("Chams");

							ImGui::Checkbox("Enabled##chams", &g_Options.chams_player_enabled); ImGui::SameLine(0, -20); ImGuiEx::ColorEdit4a("Enemy Visible ", &g_Options.color_chams_player_enemy_visible);
							ImGui::Checkbox("Visible shine##chams_enemies_visible_shine", &g_Options.player_enemies_shine);
							ImGui::SameLine(0, -20);
							ImGuiEx::ColorEdit4("##color_chams_enemies_visible_shine", &g_Options.player_enemy_visible_shine);
							ImGui::Checkbox("Team Check", &g_Options.chams_player_enemies_only);
							ImGui::Checkbox("Wireframe", &g_Options.chams_player_wireframe);
							ImGui::Checkbox("Occluded", &g_Options.chams_player_ignorez); ImGui::SameLine(0, -20); ImGuiEx::ColorEdit4a("Enemy Occluded ", &g_Options.color_chams_player_enemy_occluded);

							ImGui::Combo("##Flat", &g_Options.chams_player_flat, "Normal\0Flat \0");

							ImGui::Checkbox("Glass", &g_Options.chams_player_glass);
							ImGuiEx::ColorEdit4("Ally (Visible)", &g_Options.color_chams_player_ally_visible);
							ImGuiEx::ColorEdit4("Ally (Occluded)", &g_Options.color_chams_player_ally_occluded);
						}
						ImGui::EndGroup();
					}
					ImGui::EndChild();

					ImGui::BeginChild("Glow", ImVec2(360, 310));
					{
						ImGui::BeginGroup();
						{
							ImGui::Separator("Glow");

							ImGui::Checkbox("Enabled", &g_Options.glow_enabled);
							ImGui::SameLine(0, -20);
							ImGuiEx::ColorEdit4a("##Enemy", &g_Options.color_glow_enemy);
							ImGui::Checkbox("Occluded", &g_Options.glow_enemiesOC);
							ImGui::SameLine(0, -20);
							ImGuiEx::ColorEdit4a("##color_glow_enemiesOC", &g_Options.color_glow_enemyOC);
							const char* glow_enemies_type[] = {
								"Outline outer",
								"Pulse",
								"Outline inner"
							};
							if (ImGui::BeginCombo("##glow_enemies_type", glow_enemies_type[g_Options.glow_enemies_type], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(glow_enemies_type); i++)
								{
									if (ImGui::Selectable(glow_enemies_type[i], i == g_Options.glow_enemies_type))
										g_Options.glow_enemies_type = i;
								}
								ImGui::EndCombo();
							}
						}
						ImGui::EndGroup();
					}
					ImGui::EndChild();
				}
				ImGui::EndGroup();
				break;
			case 2:
				ImGui::BeginChild("Misc", ImVec2(350, 565));
				{
					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;
					ImGui::BeginGroup();
					{
						ImGui::Separator("Misc");
						ImGui::Checkbox("Rank reveal", &g_Options.misc_showranks);
						ImGui::Checkbox("Spectator list", &g_Options.spectator_list);
						ImGui::Checkbox("Watermark##hc", &g_Options.misc_watermark);
						ImGui::Checkbox("Auto accept", &g_Options.autoaccept);
						ImGui::Checkbox("No flash", &g_Options.no_flash);
						ImGui::Checkbox("No smoke", &g_Options.no_smoke);
						ImGui::Checkbox("Sniper crosshair", &g_Options.sniper_xhair);
						ImGui::Checkbox("Third Person", &g_Options.thirdperson); ImGui::SameLine(group_w - 50); ImGui::Hotkey(" ", &g_Options.thirdperson_key);
							if (g_Options.thirdperson)
							ImGui::SliderFloat("Distance", &g_Options.thirdperson_dist, 0.f, 250.f);
						ImGui::Checkbox("Aspect Ratio Changer", &g_Options.aspectratio);
							if (g_Options.aspectratio)
							ImGui::SliderFloat("Aspect Ratio Changer", &g_Options.aspectvalue, 0.f, 10.f);
					}
					ImGui::EndGroup();
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginChild("Movement", ImVec2(360, 565));
				{
					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;
					ImGui::BeginGroup();
					{
						ImGui::Separator("Movement");
						ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
						ImGui::Checkbox("Auto strafe", &g_Options.autostrafe); ImGui::SameLine(group_w - 50);     ImGui::Hotkey(" ", &g_Options.AutoStafe_key);
						ImGui::Checkbox("Edge bug", &g_Options.edge_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("  ", &g_Options.edge_bug_key);
						if (g_Options.edge_bug)
							ImGui::Checkbox("Edge bug detect", &g_Options.ebdetection);
						ImGui::Checkbox("Jump bug", &g_Options.jump_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("   ", &g_Options.jump_bug_key);
						ImGui::Checkbox("Edge jump", &g_Options.edgejump.enabled); ImGui::SameLine(group_w - 50); ImGui::Hotkey("    ", &g_Options.edgejump.hotkey);
						ImGui::Checkbox("LJ helper", &g_Options.edgejump.edge_jump_duck_in_air);
						ImGui::Checkbox("Blockbot", &g_Options.blockbot); ImGui::SameLine(group_w - 50); ImGui::Hotkey("     ", &g_Options.bbkey);
						ImGui::Checkbox("Velocity", &g_Options.Velocity);
						ImGui::SameLine(group_w - 22);
						ImGuiEx::ColorEdit4("##Velocity", &g_Options.Velocitycol);
						if (ImGui::BeginCombo("##Velocity", "Velocity", ImGuiComboFlags_NoArrowButton))
						{
							ImGui::Selectable("Outline", &g_Options.outline, ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable("Last jump", &g_Options.lastjump, ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable("Last jump outline", &g_Options.lastjumpoutline, ImGuiSelectableFlags_DontClosePopups);

							ImGui::EndCombo();
						}
					}
					ImGui::EndGroup();
				}
				ImGui::EndChild();
				break;
			case 3:
				ImGui::BeginChild("Config", ImVec2(350, 565));
				{
					ImGui::BeginGroup();
					{
						ImGui::Separator("Config");

						static int selected = 0;
						static char cfgName[64];

						std::vector<std::string> cfgList;
						ReadDirectory(g_Options.folder, cfgList);
						ImGui::PushItemWidth(150.f);
						if (!cfgList.empty())
						{
							if (ImGui::BeginCombo("##SelectConfig", cfgList[selected].c_str(), ImGuiComboFlags_NoArrowButton))
							{
								for (size_t i = 0; i < cfgList.size(); i++)
								{
									if (ImGui::Selectable(cfgList[i].c_str(), i == selected))
										selected = i;
								}
								ImGui::EndCombo();
							}
							if (ImGui::Button("Save config"))
								g_Options.SaveSettings(cfgList[selected]);
							//ImGui::SameLine();
							if (ImGui::Button("Load Config"))
								g_Options.LoadSettings(cfgList[selected]);
							//ImGui::SameLine();
							if (ImGui::Button("Delete config"))
							{
								g_Options.DeleteSettings(cfgList[selected]);
								selected = 0;
							}
							//	ImGui::Separator();
						}
						ImGui::Spacing();
						ImGui::SameLine();
						ImGui::InputText("##configname", cfgName, 24);
						if (ImGui::Button("Create config"))
						{
							if (strlen(cfgName))
								g_Options.SaveSettings(cfgName + std::string(".ini"));
						}
						ImGui::PopItemWidth();
					}
					ImGui::EndGroup();
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::BeginChild("Menu customization", ImVec2(360, 250));
					{
						ImGui::BeginGroup();
						{
							ImGui::Separator("Customization");

							ImGuiEx::ColorEdit3("Menu color", &g_Options.menu_color);
						}
						ImGui::EndGroup();
					}
					ImGui::EndChild();

					ImGui::BeginChild("Misc##config", ImVec2(360, 310));
					{
						ImGui::BeginGroup();
						{
							ImGui::Separator("Misc");
						}
						ImGui::EndGroup();
					}
					ImGui::EndChild();
				}
				ImGui::EndGroup();
				break;

			case 4:
				ImGui::BeginGroup();
				{
					static std::string selected_weapon_name = "";
					static std::string selected_skin_name = "";
					static auto definition_vector_index = 0;
					auto& entries = g_Options.changers.skin.m_items;

					ImGui::BeginChild("Weapon", { 350, 565 });
					{
						{
							for (size_t w = 0; w < k_weapon_names.size(); w++)
							{
								switch (w)
								{
								case 0:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "knife");
									break;
								case 2:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "glove");
									break;
								case 4:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "pistols");
									break;
								case 14:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "semi-rifle");
									break;
								case 21:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "rifle");
									break;
								case 28:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "sniper-rifle");
									break;
								case 32:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "machingun");
									break;
								case 34:
									ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "shotgun");
									break;
								}

								if (ImGui::Selectable(k_weapon_names[w].name, definition_vector_index == w))
								{
									definition_vector_index = w;
								}
							}
						}


					}
					ImGui::EndChild();

					ImGui::SameLine(0, 8);

					ImGui::BeginGroup();
					{
						ImGui::BeginChild("Skin", { 360, 350 });
						{


							ImGui::Spacing();
							ImGui::Spacing();
							auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
							auto& satatt = g_Options.changers.skin.statrack_items[k_weapon_names[definition_vector_index].definition_index];
							selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
							selected_entry.definition_vector_index = definition_vector_index;
							ImGui::Separator("Weapon Misc");
							//ImGui::Checkbox("skin preview", &g_Options.changers.skin.skin_preview);
							ImGui::Checkbox("stattrak##2", &selected_entry.stat_trak);
							ImGui::InputInt("seed", &selected_entry.seed);
							ImGui::InputInt("stattrak", &satatt.statrack_new.counter);
							ImGui::SliderFloat("wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);
							ImGui::Separator("Weapon Paint");
							if (selected_entry.definition_index == WEAPON_KNIFE || selected_entry.definition_index == WEAPON_KNIFE_T)
							{
								ImGui::PushItemWidth(160.f);

								ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
								{
									*out_text = k_knife_names.at(idx).name;
									return true;
								}, nullptr, k_knife_names.size(), 10);
								selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

							}
							else if (selected_entry.definition_index == GLOVE_T_SIDE || selected_entry.definition_index == GLOVE_CT_SIDE)
							{
								ImGui::PushItemWidth(160.f);

								ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
								{
									*out_text = k_glove_names.at(idx).name;
									return true;
								}, nullptr, k_glove_names.size(), 10);
								selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
							}
							else {
								static auto unused_value = 0;
								selected_entry.definition_override_vector_index = 0;
							}

							if (selected_entry.definition_index != GLOVE_T_SIDE &&
								selected_entry.definition_index != GLOVE_CT_SIDE &&
								selected_entry.definition_index != WEAPON_KNIFE &&
								selected_entry.definition_index != WEAPON_KNIFE_T)
							{
								selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
							}
							else
							{
								if (selected_entry.definition_index == GLOVE_T_SIDE ||
									selected_entry.definition_index == GLOVE_CT_SIDE)
								{
									selected_weapon_name = k_glove_names_preview.at(selected_entry.definition_override_vector_index).name;
								}
								if (selected_entry.definition_index == WEAPON_KNIFE ||
									selected_entry.definition_index == WEAPON_KNIFE_T)
								{
									selected_weapon_name = k_knife_names_preview.at(selected_entry.definition_override_vector_index).name;
								}
							}
							if (skins_parsed)
							{

								static char filter_name[32];
								std::string filter = filter_name;

								bool is_glove = selected_entry.definition_index == GLOVE_T_SIDE ||
									selected_entry.definition_index == GLOVE_CT_SIDE;

								bool is_knife = selected_entry.definition_index == WEAPON_KNIFE ||
									selected_entry.definition_index == WEAPON_KNIFE_T;

								int cur_weapidx = 0;
								if (!is_glove && !is_knife)
								{
									cur_weapidx = k_weapon_names[definition_vector_index].definition_index;
									selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
								}
								else
								{
									if (selected_entry.definition_index == GLOVE_T_SIDE ||
										selected_entry.definition_index == GLOVE_CT_SIDE)
									{
										cur_weapidx = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
									}
									if (selected_entry.definition_index == WEAPON_KNIFE ||
										selected_entry.definition_index == WEAPON_KNIFE_T)
									{
										cur_weapidx = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

									}
								}


								auto weaponName = weaponnames(cur_weapidx);

								{
									if (selected_entry.definition_index != GLOVE_T_SIDE && selected_entry.definition_index != GLOVE_CT_SIDE)
									{
										if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
										{
											selected_entry.paint_kit_vector_index = -1;
											selected_entry.paint_kit_index = -1;
											selected_skin_name = "";
										}

										int lastID = ImGui::GetItemID();
										for (size_t w = 0; w < k_skins.size(); w++)
										{
											for (auto names : k_skins[w].weaponName)
											{
												std::string name = k_skins[w].name;

												if (g_Options.changers.skin.show_cur)
												{
													if (names != weaponName)
														continue;
												}

												if (name.find(filter) != name.npos)
												{
													ImGui::PushID(lastID++);

													ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(is_knife && g_Options.changers.skin.show_cur ? 6 : k_skins[w].rarity));
													{
														if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
														{
															selected_entry.paint_kit_vector_index = w;
															selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
															selected_skin_name = k_skins[w].name_short;
														}
													}
													ImGui::PopStyleColor();

													ImGui::PopID();
												}
											}
										}
									}
									else
									{
										int lastID = ImGui::GetItemID();

										if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
										{
											selected_entry.paint_kit_vector_index = -1;
											selected_entry.paint_kit_index = -1;
											selected_skin_name = "";
										}

										for (size_t w = 0; w < k_gloves.size(); w++)
										{
											for (auto names : k_gloves[w].weaponName)
											{
												std::string name = k_gloves[w].name;
												name += " | ";
												name += names;

												if (g_Options.changers.skin.show_cur)
												{
													if (names != weaponName)
														continue;
												}

												if (name.find(filter) != name.npos)
												{
													ImGui::PushID(lastID++);

													ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(6));
													{
														if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
														{
															selected_entry.paint_kit_vector_index = w;
															selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
															selected_skin_name = k_gloves[selected_entry.paint_kit_vector_index].name_short;
														}
													}
													ImGui::PopStyleColor();

													ImGui::PopID();
												}
											}
										}
									}
								}
								//	ImGui::ListBoxFooter();
							}
							else
							{
								ImGui::Text("skins parsing, wait...");
							}





						}
						ImGui::EndChild();


						ImGui::BeginChild("Player Model", { 360, 310 });
						{

							ImGui::Text("CT Model");
							ImGui::PushItemWidth(160.f);
							ImGui::Combo("##TPlayerModel", &g_Options.playerModelCT, "Default\0Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman\0Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman\0Lieutenant Rex Krikey | SEAL Frogman\0Michael Syfers | FBI Sniper\0Operator | FBI SWAT\0Special Agent Ava | FBI\0Markus Delrow | FBI HRT\0Sous-Lieutenant Medic | Gendarmerie Nationale\0Chem-Haz Capitaine | Gendarmerie Nationale\0Chef d'Escadron Rouchard | Gendarmerie Nationale\0Aspirant | Gendarmerie Nationale\0Officer Jacques Beltram | Gendarmerie Nationale\0D Squadron Officer | NZSAS\0B Squadron Officer | SAS\0Seal Team 6 Soldier | NSWC SEAL\0Buckshot | NSWC SEAL\0Lt. Commander Ricksaw | NSWC SEAL\0'Blueberries' Buckshot | NSWC SEAL\0third Commando Company | KSK\0'Two Times' McCoy | TACP Cavalry\0'Two Times' McCoy | USAF TACP\0Primeiro Tenente | Brazilian 1st Battalion\0Cmdr. Mae 'Dead Cold' Jamison | SWAT\0first Lieutenant Farlow | SWAT\0John 'Van Healen' Kask | SWAT\0Bio-Haz Specialist | SWAT\0Sergeant Bombson | SWAT\0Chem-Haz Specialist | SWAT\0Getaway Sally | The Professionals\0Number K | The Professionals\0Little Kev | The Professionals\0Safecracker Voltzmann | The Professionals\0Bloody Darryl The Strapped | The Professionals\0Sir Bloody Loudmouth Darryl | The Professionals\0Sir Bloody Darryl Royale | The Professionals\0Sir Bloody Skullhead Darryl | The Professionals\0Sir Bloody Silent Darryl | The Professionals\0Sir Bloody Miami Darryl | The Professionals\0Street Soldier | Phoenix\0Soldier | Phoenix\0Slingshot | Phoenix\0Enforcer | Phoenix\0Mr. Muhlik | Elite Crew\0Prof. Shahmat | Elite Crew\0Osiris | Elite Crew\0Ground Rebel | Elite Crew\0The Elite Mr. Muhlik | Elite Crew\0Trapper | Guerrilla Warfare\0Trapper Aggressor | Guerrilla Warfare\0Vypa Sista of the Revolution | Guerrilla Warfare\0Col. Mangos Dabisi | Guerrilla Warfare\0Arno The Overgrown | Guerrilla Warfare\0'Medium Rare' Crasswater | Guerrilla Warfare\0Crasswater The Forgotten | Guerrilla Warfare\0Elite Trapper Solman | Guerrilla Warfare\0'The Doctor' Romanov | Sabre\0Blackwolf | Sabre\0Maximus | Sabre\0Dragomir | Sabre\0Rezan The Ready | Sabre\0Rezan the Redshirt | Sabre\0Dragomir | Sabre Footsoldier\0");

							ImGui::Text("T Model");
							ImGui::PushItemWidth(160.f);
							ImGui::Combo("##CTPlayerModel", &g_Options.playerModelT, "Default\0Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman\0Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman\0Lieutenant Rex Krikey | SEAL Frogman\0Michael Syfers | FBI Sniper\0Operator | FBI SWAT\0Special Agent Ava | FBI\0Markus Delrow | FBI HRT\0Sous-Lieutenant Medic | Gendarmerie Nationale\0Chem-Haz Capitaine | Gendarmerie Nationale\0Chef d'Escadron Rouchard | Gendarmerie Nationale\0Aspirant | Gendarmerie Nationale\0Officer Jacques Beltram | Gendarmerie Nationale\0D Squadron Officer | NZSAS\0B Squadron Officer | SAS\0Seal Team 6 Soldier | NSWC SEAL\0Buckshot | NSWC SEAL\0Lt. Commander Ricksaw | NSWC SEAL\0'Blueberries' Buckshot | NSWC SEAL\0third Commando Company | KSK\0'Two Times' McCoy | TACP Cavalry\0'Two Times' McCoy | USAF TACP\0Primeiro Tenente | Brazilian 1st Battalion\0Cmdr. Mae 'Dead Cold' Jamison | SWAT\0first Lieutenant Farlow | SWAT\0John 'Van Healen' Kask | SWAT\0Bio-Haz Specialist | SWAT\0Sergeant Bombson | SWAT\0Chem-Haz Specialist | SWAT\0Getaway Sally | The Professionals\0Number K | The Professionals\0Little Kev | The Professionals\0Safecracker Voltzmann | The Professionals\0Bloody Darryl The Strapped | The Professionals\0Sir Bloody Loudmouth Darryl | The Professionals\0Sir Bloody Darryl Royale | The Professionals\0Sir Bloody Skullhead Darryl | The Professionals\0Sir Bloody Silent Darryl | The Professionals\0Sir Bloody Miami Darryl | The Professionals\0Street Soldier | Phoenix\0Soldier | Phoenix\0Slingshot | Phoenix\0Enforcer | Phoenix\0Mr. Muhlik | Elite Crew\0Prof. Shahmat | Elite Crew\0Osiris | Elite Crew\0Ground Rebel | Elite Crew\0The Elite Mr. Muhlik | Elite Crew\0Trapper | Guerrilla Warfare\0Trapper Aggressor | Guerrilla Warfare\0Vypa Sista of the Revolution | Guerrilla Warfare\0Col. Mangos Dabisi | Guerrilla Warfare\0Arno The Overgrown | Guerrilla Warfare\0'Medium Rare' Crasswater | Guerrilla Warfare\0Crasswater The Forgotten | Guerrilla Warfare\0Elite Trapper Solman | Guerrilla Warfare\0'The Doctor' Romanov | Sabre\0Blackwolf | Sabre\0Maximus | Sabre\0Dragomir | Sabre\0Rezan The Ready | Sabre\0Rezan the Redshirt | Sabre\0Dragomir | Sabre Footsoldier\0");

							if (ImGui::Button(" update skin"))
							{
								//	if (next_enb_time <= g_GlobalVars->curtime)
								{
									static auto clear_hud_weapon_icon_fn =
										reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
											Utils::PatternScan2("client.dll", "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC"));

									auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");

									if (element)
									{
										auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xa0);
										if (hud_weapons != nullptr)
										{

											if (*hud_weapons->get_weapon_count())
											{
												for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
													i = clear_hud_weapon_icon_fn(hud_weapons, i);

												typedef void(*ForceUpdate) (void);
												static ForceUpdate FullUpdate = (ForceUpdate)Utils::PatternScan2("engine.dll", "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");
												FullUpdate();

												g_ClientState->ForceFullUpdate();
											}
										}
									}

									//next_enb_time = g_GlobalVars->curtime + 1.f;
								}
							}

						}
						ImGui::EndChild();
					}
					ImGui::EndGroup();


				}
				ImGui::EndGroup();
				break;
			case 5:
				ImGui::BeginChild("HvH", { 350, 565 });
				{
					ImGui::BeginGroup();
					{
						ImGui::Separator("Under Developement");
						ImGui::Checkbox("Resolver", &g_Options.resolver);
					}
					ImGui::EndGroup();
				}
				ImGui::EndChild();

				break;
			
			}
		}
		ImGui::EndGroup();
	}
	ImGui::End();
}

void Menu::Toggle()
{
	_visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	ImGui::StyleColorsDark();

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.f, 0.f, 0.f, 1.f);
}

void Menu::UpdateStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_SliderGrabActive] = ImVec4(((float)g_Options.menu_color.r() / 255), ((float)g_Options.menu_color.g() / 255), ((float)g_Options.menu_color.b() / 255), 1.00f);
}