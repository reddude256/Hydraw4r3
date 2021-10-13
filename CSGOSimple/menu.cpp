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
#define RGBA_TO_FLOAT(r,g,b,a) (float)r/255.0f, (float)g/255.0f, (float)b/255.0f, (float)a/255.0f

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

static int weapon_index = 7;
static int weapon_vector_index = 0;
struct WeaponName_t {
	constexpr WeaponName_t(int32_t definition_index, const char* name) :
		definition_index(definition_index),
		name(name) {
	}

	int32_t definition_index = 0;
	const char* name = nullptr;
};

std::vector< WeaponName_t> WeaponNames = {
	{61, "Usp-s"},
	{32, "P2000"},
	{4, "Glock-18"},
	{2, "Dual berettas"},
	{36, "P250"},
	{3, "Five-Seven"},
	{30, "Tec-9"},
	{63, "Cz75a"},
	{64, "R8 revolver"},
	{1, "Deagle"},

	{34, "Mp9"},
	{17, "Mac-10"},
	{23, "Mp5-sd"},
	{33, "Mp7"},
	{24, "Ump-45"},
	{19, "P90"},
	{26, "PP-Bizon"},

	{7, "Ak-47"},
	{60, "M4a1-s"},
	{16, "M4a4"},
	{8, "Aug"},
	{39, "Sg553"},
	{10, "Famas"},
	{13, "Galil"},

	{40, "Ssg08"},
	{38, "Scar-20"},
	{9, "Awp"},
	{11, "G3sg1"},

	{14, "M249"},
	{28, "Negev"},

	{27, "Mag-7"},
	{35, "Nova"},
	{29, "Sawed-off"},
	{25, "Xm1014"},
};

void RenderCurrentWeaponButton() {
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;
	short wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
	auto wpn_it = std::find_if(WeaponNames.begin(), WeaponNames.end(), [wpn_idx](const WeaponName_t& a) {
		return a.definition_index == wpn_idx;
		});
	if (wpn_it != WeaponNames.end()) {
		weapon_index = wpn_idx;
		weapon_vector_index = std::abs(std::distance(WeaponNames.begin(), wpn_it));
	}
}

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
		if (ImGui::ColorEdit4(label, clr, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
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
bool Tab(const char* label, const ImVec2& size_arg, bool state)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);
	if (pressed)
		ImGui::MarkItemEdited(id);

	ImGui::RenderFrame(bb.Min, bb.Max, state ? ImColor(0, 0, 0) : ImColor(0, 0, 0), true, style.FrameRounding);
	window->DrawList->AddRect(bb.Min, bb.Max, ImColor(0, 0, 0));
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	if (state)
	{
		window->DrawList->AddLine(bb.Min, bb.Min + ImVec2(9, 0), ImColor(80, 80, 160), 1);
		window->DrawList->AddLine(bb.Min, bb.Min + ImVec2(0, 9), ImColor(80, 80, 160), 1);

		window->DrawList->AddLine(bb.Max - ImVec2(0, 1), bb.Max - ImVec2(10, 1), ImColor(80, 80, 160), 1);
		window->DrawList->AddLine(bb.Max - ImVec2(1, 1), bb.Max - ImVec2(1, 10), ImColor(80, 80, 160), 1);
	}

	return pressed;
}

void SpectatorList()
{
	if (!g_Options.spectator_list)
		return;

	int specs = 0;
	std::string spect = "";

	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
		int localIndex = g_EngineClient->GetLocalPlayer();
		C_BasePlayer* pLocalEntity = C_BasePlayer::GetPlayerByIndex(localIndex);
		if (pLocalEntity) {
			for (int i = 0; i < g_EngineClient->GetMaxClients(); i++) {
				C_BasePlayer* pBaseEntity = C_BasePlayer::GetPlayerByIndex(i);
				if (!pBaseEntity)										     continue;
				if (pBaseEntity->m_iHealth() > 0)							 continue;
				if (pBaseEntity == pLocalEntity)							 continue;
				if (pBaseEntity->IsDormant())								 continue;
				if (pBaseEntity->m_hObserverTarget() != pLocalEntity)		 continue;
				player_info_t pInfo;
				g_EngineClient->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
				if (pInfo.ishltv) continue;

				spect += pInfo.szName;
				spect += "\n";
				specs++;
			}
		}
	}
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f)); //ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar // ImVec(0, 0)
	if (ImGui::Begin("toxicware", nullptr, ImVec2(ImGui::GetCursorScreenPos()), 0.4F, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
		if (specs > 0) spect += "\n";

		ImVec2 size = ImGui::CalcTextSize(spect.c_str());
		ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
		ImGui::Text(spect.c_str());
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;
	SpectatorList();
	if (!_visible)
		return;
	auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | NULL | NULL | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | NULL | NULL | NULL;

	static int tab = 0;

	ImGui::SetNextWindowSize({ 560.000000f,360.000000f }); //560.000000f,360.000000f

	ImGui::Begin("Edited", nullptr, flags);
	{
		ImVec2 p = ImGui::GetWindowPos();
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 550, p.y + 350), ImColor(0, 0, 0, 255), 0, 15); // main frame //x + 550, p.y + 350 //0.000000f, 0.000000f, 0.000000f, 0.4f
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 0, p.y + 0), ImVec2(p.x + 560, p.y + 360), ImColor(80, 80, 160, 255), 0, 15, 1.000000);  // main frame //x + 560, p.y + 360 //0.000000f, 0.000000f, 0.000000f, 0.639216f), 0, 15, 1.000000
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 550, p.y + 350), ImColor(80, 80, 160, 255), 0, 15, 1.000000);  // main frame // x + 550, p.y + 350
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 15, p.y + 60), ImVec2(p.x + 545, p.y + 345), ImColor(0, 0, 0, 255), 0, 15); // main frame // x + 545, p.y + 345
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 15, p.y + 60), ImVec2(p.x + 545, p.y + 345), ImColor(80, 80, 160, 255), 0, 15, 1.000000);  // main frame // x + 545, p.y + 345

		ImGui::SetCursorPos({ 15,17 });
		if (Tab("Legitbot", { 125,35 }, tab == 0))
			tab = 0;

		ImGui::SetCursorPos({ 15 + 125 + 10,17 });
		if (Tab("Visuals", { 125,35 }, tab == 1))
			tab = 1;

		ImGui::SetCursorPos({ 15 + 250 + 20,17 });
		if (Tab("Misc", { 125,35 }, tab == 2))
			tab = 2;

		ImGui::SetCursorPos({ 15 + 375 + 30,17 });
		if (Tab("Skins", { 125,35 }, tab == 3))
			tab = 3;

		if (tab == 0)
		{
			static int definition_index = WEAPON_INVALID;

			auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
			if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
				definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
			else
				definition_index = WEAPON_INVALID;
			if (definition_index == WEAPON_INVALID)definition_index = WEAPON_DEAGLE;
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{
				ImGui::Separator("Legitbot");
				auto settings = &g_Options.weapons[definition_index].legit;

				/*ImGui::PushItemWidth(-1);
				if (ImGui::Combo("##Weapon", &weapon_vector_index, [](void* data, int32_t idx, const char** out_text)
				{auto vec = reinterpret_cast<std::vector< WeaponName_t >*>(data);	*out_text = vec->at(idx).name; return true; },
					(void*)(&WeaponNames), WeaponNames.size())) {
					weapon_index = WeaponNames[weapon_vector_index].definition_index;
				}*/

				ImGui::Checkbox("Enabled", &settings->enabled);
				//ImGui::Checkbox("Friendly fire", &settings->deathmatch);
				ImGui::Combo("Silent", &settings->silent2, "Off\0Silent \0PSilent\0");
				ImGui::Checkbox("Off when flashed", &settings->flash_check);
				ImGui::Checkbox("Off when smoked", &settings->smoke_check);
				ImGui::Checkbox("Auto pistol", &settings->autopistol);

				if (ImGui::BeginCombo("##hitbox_filter", "Hitboxes", ImGuiComboFlags_NoArrowButton))
				{
					ImGui::Selectable("Head", &settings->hitboxes.head, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("Chest", &settings->hitboxes.chest, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("Hands", &settings->hitboxes.hands, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("Legs", &settings->hitboxes.legs, ImGuiSelectableFlags_DontClosePopups);

					ImGui::EndCombo();
				}


			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				auto settings = &g_Options.weapons[definition_index].legit;
				ImGui::Separator("Misc");

				ImGui::Text("  Fov");
				ImGui::Spacing();
				ImGui::SliderFloat("##Fov", &settings->fov, 0.f, 180.f, "%.f");
				ImGui::Spacing();
				if (settings->silent2) {
					ImGui::Text("  Silent fov");
					ImGui::Spacing();
					ImGui::SliderFloat("##Silentfov", &settings->silent_fov, 0.f, 180.f, "%.f");
				}
				ImGui::Text("  Smooth");
				ImGui::Spacing();
				ImGui::SliderFloat("##Smooth", &settings->smooth, 1.f, 20.f, "%.f");

				ImGui::Separator("Delays");


				if (!settings->silent2) {
					ImGui::Text("  Shot delay");
					ImGui::Spacing();
					ImGui::SliderInt("##Shotdelay", &settings->shot_delay, 0, 1000, "%i");
				}
				ImGui::Text("  Kill delay");
				ImGui::Spacing();
				ImGui::SliderInt("##Killdelay", &settings->kill_delay, 0, 1000, "%i");

				//ImGui::Separator("Memes");
				//ImGui::Checkbox("Local rng exploit", &g_Options.misc_rng_exploit);

				ImGui::Separator("RCS");

				ImGui::Checkbox("Enabled##rcs", &settings->rcs.enabled);

				const char* rcs_types[] = {
					"Type: Standalone",
					"Type: Aim"
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
				//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
				ImGui::SliderInt("  X", &settings->rcs.x, 0, 100, "%i");
				ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
				ImGui::SliderInt("  Y", &settings->rcs.y, 0, 100, "%i");
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
			{
				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				auto settings = &g_Options.weapons[definition_index].legit;



				//ImGui::Separator("Auto wall ");

				//ImGui::Checkbox("Enabled##autowall", &settings->autowall);
				//ImGui::Spacing();
				//ImGui::SameLine();
				//ImGui::Text("Min Damage");
				//ImGui::Spacing();
				//ImGui::SliderInt("##minDamage", &settings->autowall.min_damage, 1, 100, "%i");



				ImGui::Separator("Auto fire");
				ImGui::Checkbox("Enabled##autofire", &settings->autofire.enabled);
				ImGui::SameLine(group_w - 50);
				ImGui::Hotkey("##autofire", &settings->autofire.hotkey);

				ImGui::Separator("BackTrack");
				ImGui::NewLine();
				ImGui::SliderInt("Backtrack", &settings->backtrack.ticks, 1, 12, "%i");

				/*ImGui::Separator("Triggerbot");

				ImGui::Checkbox("Enabled##trigger", &settings->trigger_enable);
				//ImGui::Checkbox("Friendly Fire##trigger", &settings->trigger_deatchmatch);
				ImGui::Checkbox("On key##trigger", &settings->trigger_onkey);
				if (trigger_onkey) {
					ImGui::SameLine();
					ImGui::HotKey("##trigger", &trigger_keybind);
				}
				ImGui::Checkbox("Check Smoke##trigger", &settings->trigger_check_smoke);
				ImGui::Checkbox("Check Flash##trigger", &settings->trigger_check_flash);
				ImGui::SliderFloat("Hitchance", &settings->trigger_hit_chance, 0.f, 100.f);
				ImGui::SliderFloat("Delay", &settings->trigger_delay, 0.f, 150.f);*/
			}
			ImGui::EndChild();
		}
		else if (tab == 1)
		{
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{
				ImGui::Separator("ESP");
				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				//	ImGui::Checkbox("Enabled",&g_Options.esp_enabled);
					//ImGui::Checkbox("Team check",&g_Options.esp_enemies_only);
				ImGui::Checkbox("Boxes", &g_Options.esp_player_boxes); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("Enemies Visible   ", &g_Options.color_esp_enemy_visible);
				ImGui::Checkbox("XQZ ", &g_Options.esp_player_boxesOccluded); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("Enemies Occluded      ", &g_Options.color_esp_enemy_occluded);

				ImGui::Checkbox("Names", &g_Options.esp_player_names);
				ImGui::Checkbox("Health", &g_Options.esp_player_health);
				ImGui::Checkbox("Armour", &g_Options.esp_player_armour);
				//ImGui::Checkbox("Skeleton", &g_Options.esp_player_skeleton); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("Skeleton color   ", &g_Options.color_esp_enemy_skeleton);
				ImGui::Checkbox("Weapon", &g_Options.esp_player_weapons);
				ImGui::Checkbox("Snaplines", &g_Options.esp_player_snaplines);
				ImGui::Checkbox("Dropped Weapons", &g_Options.esp_dropped_weapons);// ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("##Weapons     ", &g_Options.color_esp_weapons);
				//ImGui::Checkbox("Planted C4", &g_Options.esp_planted_c4);
				//ImGuiEx::ColorEdit4("Allies Visible",&g_Options.color_esp_ally_visible);
				//ImGuiEx::ColorEdit4("Enemies Visible",&g_Options.color_esp_enemy_visible);
				//ImGuiEx::ColorEdit4("Allies Occluded",&g_Options.color_esp_ally_occluded);
				//ImGuiEx::ColorEdit4("Enemies Occluded",&g_Options.color_esp_enemy_occluded);
				//ImGuiEx::ColorEdit4("Dropped Weapons",&g_Options.color_esp_weapons);
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				ImGui::Separator("Chams");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Enabled ", &g_Options.chams_player_enabled); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4a("Enemy Visible ", &g_Options.color_chams_player_enemy_visible);
				ImGui::Checkbox("Visible shine##chams_enemies_visible_shine", &g_Options.player_enemies_shine);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4("##color_chams_enemies_visible_shine", &g_Options.player_enemy_visible_shine);
				//ImGui::Checkbox("Team Check",&g_Options.chams_player_enemies_only);
				//ImGui::Checkbox("Wireframe",&g_Options.chams_player_wireframe);

				ImGui::Checkbox("XQZ  ", &g_Options.chams_player_ignorez); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4a("Enemy Occluded ", &g_Options.color_chams_player_enemy_occluded);

				//ImGui::Checkbox("Flat",&g_Options.chams_player_flat);
				ImGui::Combo("##Flat", &g_Options.chams_player_flat, "Normal\0Flat\0Glass\0");

				//ImGui::Checkbox("Glass",&g_Options.chams_player_glass);
				//ImGuiEx::ColorEdit4("Ally (Visible)",&g_Options.color_chams_player_ally_visible);
				//ImGuiEx::ColorEdit4("Ally (Occluded)",&g_Options.color_chams_player_ally_occluded);

				//ImGui::Checkbox("Arms enabled", &g_Options.chams_arms_enabled);

				ImGui::Separator("Viewmodel");
				ImGui::Checkbox("Custom viewmodel", &g_Options.custom_viewmodel);
				if (g_Options.custom_viewmodel) {
					ImGui::SliderInt("Viewmodel FOV", &g_Options.viewmodel_fov, 68, 120);
					ImGui::SliderInt("Offset X", &g_Options.viewmodel_offset_x, -20, 20);
					ImGui::SliderInt("Offset Y", &g_Options.viewmodel_offset_y, -20, 20);
					ImGui::SliderInt("Offset Z", &g_Options.viewmodel_offset_z, -20, 20);
				}
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
			{
				ImGui::Separator("Glow");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Enabled", &g_Options.glow_enabled);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4a("##Enemy   ", &g_Options.color_glow_enemy);
				ImGui::Checkbox("XQZ   ", &g_Options.glow_enemiesOC);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4a("##color_glow_enemiesOC   ", &g_Options.color_glow_enemyOC);
				const char* glow_enemies_type[] = {
					"Outline outer",
					"Pulse",
					"Outline inner"
				};
				/*if (ImGui::BeginCombo("##glow_enemies_type", glow_enemies_type[g_Options.glow_enemies_type], ImGuiComboFlags_NoArrowButton))
				{
					for (int i = 0; i < IM_ARRAYSIZE(glow_enemies_type); i++)
					{
						if (ImGui::Selectable(glow_enemies_type[i], i == g_Options.glow_enemies_type))
							g_Options.glow_enemies_type = i;
					}

					ImGui::EndCombo();
				}*/
				ImGui::Separator("Other");
				//ImGui::Checkbox("Nightmode", &g_Options.nightmode);
				//ImGui::Checkbox("R_drawclipbrushes", &g_Options.clipbrushes);
				//ImGui::Checkbox("Inverse ragdoll gravity", &g_Options.ragdollgravity);
				ImGui::NewLine();
				ImGui::Checkbox("Aspect ratio", &g_Options.aspectratio);
				if (g_Options.aspectratio) {
					ImGui::SliderFloat("Ratio", &g_Options.aspectvalue, 0.f, 10.f);
				}
				ImGui::Checkbox("Skybox changer", &g_Options.skyboxchanger);
				if (g_Options.skyboxchanger) {
					ImGui::Combo("Skyboxes", &g_Options.skybox, "Baggage\0Monastery\0Italy/OldInferno\00Aztec\0Vertigo\0Daylight\0Daylight (2)\0Clouds\0Clouds (2)\0Gray\0Clear\0Canals\0Cobblestone\0Assault\0Clouds (Dark)\0Night\0Night (2)\0Night (Flat)\0Dusty\0Rainy\0");
				}

				//ImGui::Checkbox("FOG editor", &g_Options.esp_world_fog); ImGui::SameLine(125); ImGuiEx::ColorEdit4("##fogcolor", &g_Options.fog_color);
				//if (g_Options.esp_world_fog) {
				//	ImGui::NewLine();
				//	ImGui::SliderFloat("FOG START", &g_Options.esp_world_fog_start, 0, 3000, "%.0f");
				//	ImGui::SliderFloat("FOG END", &g_Options.esp_world_fog_end, 0, 5000, "%.0f");
				//	ImGui::SliderFloat("FOG DESTINY", &g_Options.esp_world_fog_destiny, 0, 1);
				//}
				ImGui::Checkbox("Third person", &g_Options.thirdperson);
				ImGui::SameLine(group_w - 20); ImGui::Hotkey("     ", &g_Options.thirdperson_key);
				if (&g_Options.thirdperson)
					ImGui::SliderInt("Distance", &g_Options.thirdperson_dist, 0.f, 250.f);
				//ImGui::Checkbox("Hitmarker", &g_Options.hitmarker);
			}
			ImGui::EndChild();
		}
		else if (tab == 2)
		{
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{
				ImGui::Separator("General");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Rank reveal", &g_Options.misc_showranks);
				//ImGui::Checkbox("Sv_pure bypass", &g_Options.misc_pure_bypass);
				//ImGui::Checkbox("Engine radar", &g_Options.engineradar);
				ImGui::Checkbox("Spectator list", &g_Options.spectator_list);
				ImGui::Checkbox("Watermark##hc", &g_Options.misc_watermark);
				//if (g_Options.misc_watermark) {
					//ImGui::SameLine();
					//ImGui::Checkbox("Rainbow", &g_Options.misc_watermark_rainbow);
				//}
				//ImGui::Checkbox("Clantag", &g_Options.misc_clantag);
				//ImGui::Checkbox("Chat spam", &g_Options.misc_chatspam);
				//ImGui::Checkbox("Radio spam", &g_Options.radio_spam);
				ImGui::Checkbox("Velocity ind", &g_Options.Velocity);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4("##Velocity", &g_Options.Velocitycol);
				ImGui::Spacing();
				if (ImGui::BeginCombo("##Velocity", "Velocity", ImGuiComboFlags_NoArrowButton))
				{
					ImGui::Selectable("Outline", &g_Options.outline, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("Last jump", &g_Options.lastjump, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("Last jump outline", &g_Options.lastjumpoutline, ImGuiSelectableFlags_DontClosePopups);

					ImGui::EndCombo();
				}
				ImGui::Checkbox("Velocity graph", &g_Options.bVelocityGraph);
				ImGui::SameLine(group_w - 20);
				ImGuiEx::ColorEdit4("##Graph", &g_Options.colorgraph);
				if (g_Options.bVelocityGraph) {
					ImGui::Spacing();
					ImGui::SliderInt("graph Y add", &g_Options.iYAdditive, -500, 500);
					ImGui::SliderInt("graph X add", &g_Options.iXAdditive, -500, 500);
				};
				//ImGui::Checkbox("Show keypresses", &g_Options.esp_show_keypresses); //ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("##Keypresses", &g_Options.Keypressescol);
				//ImGui::Checkbox("Indicators", &g_Options.esp_show_indicators); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("##Indicators", &g_Options.Indicatorscol);
				ImGui::Checkbox("Auto accept", &g_Options.autoaccept);
				ImGui::Checkbox("No flash", &g_Options.no_flash);
				ImGui::Checkbox("Wireframe smoke", &g_Options.no_smoke);
				ImGui::Checkbox("Sniper crosshair", &g_Options.sniper_xhair);

			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				ImGui::Separator("Movement");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
				//ImGui::Checkbox("Infinite duck", &g_Options.misc_infinite_duck);
				ImGui::Checkbox("Auto strafe", &g_Options.autostrafe); ImGui::SameLine(group_w - 50);     ImGui::Hotkey(" ", &g_Options.AutoStafe_key);
				ImGui::Checkbox("Edge bug", &g_Options.edge_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("  ", &g_Options.edge_bug_key);
				ImGui::Spacing();
				if (ImGui::BeginCombo("##Edge bug", "Edge bug", ImGuiComboFlags_NoArrowButton))
				{
				//	ImGui::Selectable("Eb velocity booster", &g_Options.autostrafe, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("Smart eb (test)", &g_Options.smart_eb, ImGuiSelectableFlags_DontClosePopups);
				//	ImGui::Selectable("Fake edge bug", &g_Options.localeb, ImGuiSelectableFlags_DontClosePopups);
				
					ImGui::EndCombo();
				}
				ImGui::Checkbox("Show eb detection", &g_Options.ebdetection);
				if (g_Options.ebdetection) {
					if (ImGui::BeginCombo("###Decetion", "Eb detection", ImGuiComboFlags_NoArrowButton))
					{
						ImGui::Selectable("Chat", &g_Options.eb_detection_chat, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Effect", &g_Options.eb_detection_effect, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("Sound", &g_Options.eb_detection_sound, ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
				}
				ImGui::Checkbox("Jump bug", &g_Options.jump_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("   ", &g_Options.jump_bug_key);
				ImGui::Checkbox("Edge jump", &g_Options.edgejump.enabled); ImGui::SameLine(group_w - 50); ImGui::Hotkey("    ", &g_Options.edgejump.hotkey);
				if (g_Options.edgejump.enabled)
					ImGui::Checkbox("LJ helper", &g_Options.edgejump.edge_jump_duck_in_air);

				ImGui::NewLine();
				ImGui::Separator("Griefing");
				//ImGui::Checkbox("Radio spam", &g_Options.radio_spam);
				ImGui::Checkbox("Blockbot", &g_Options.blockbot); ImGui::SameLine(group_w - 50); ImGui::Hotkey("     ", &g_Options.bbkey);
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
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
					if (ImGui::Button(" Save   Config"))
						g_Options.SaveSettings(cfgList[selected]);
					//ImGui::SameLine();
					if (ImGui::Button(" Load   Config"))
						g_Options.LoadSettings(cfgList[selected]);
					//ImGui::SameLine();
					if (ImGui::Button(" Delete Config"))
					{
						g_Options.DeleteSettings(cfgList[selected]);
						selected = 0;
					}
					//	ImGui::Separator();
				}
				ImGui::Spacing();
				ImGui::SameLine();
				ImGui::InputText("##configname", cfgName, 24);
				//ImGui::SameLine();
				if (ImGui::Button(" Create Config"))
				{
					if (strlen(cfgName))
						g_Options.SaveSettings(cfgName + std::string(".ini"));
				}
				ImGui::PopItemWidth();
			}
			ImGui::EndChild();
		}
		else if (tab == 3)
		{
			static std::string selected_weapon_name = "";
			static std::string selected_skin_name = "";
			static auto definition_vector_index = 0;
			auto& entries = g_Options.changers.skin.m_items;
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{
				/*	ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SameLine();*/
					//	ImGui::ListBoxHeader("##sjinstab",ImVec2(155,245));
				ImGui::Spacing();
				ImGui::Spacing();

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
				//ImGui::ListBoxFooter();

			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				ImGui::Spacing();
				ImGui::Spacing();
				auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
				auto& satatt = g_Options.changers.skin.statrack_items[k_weapon_names[definition_vector_index].definition_index];
				selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
				selected_entry.definition_vector_index = definition_vector_index;
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
						//selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
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

					/*	ImGui::InputText("name filter [?]", filter_name, sizeof(filter_name));
						if (ImGui::ItemsToolTipBegin("##skinfilter"))
						{
							ImGui::Checkbox("show skins for selected weapon", &g_Options.changers.skin.show_cur);
							ImGui::ItemsToolTipEnd();
						}*/

					auto weaponName = weaponnames(cur_weapidx);
					/*ImGui::Spacing();

					ImGui::Spacing();
					ImGui::SameLine();*/
					//ImGui::ListBoxHeader("##sdsdadsdadas", ImVec2(155, 245));
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
									//name += " | ";
									//name += names;

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
				//ImGui::Checkbox("skin preview", &g_Options.changers.skin.skin_preview);
				ImGui::Checkbox("stattrak##2", &selected_entry.stat_trak);
				ImGui::InputInt("seed", &selected_entry.seed);
				ImGui::InputInt("stattrak", &satatt.statrack_new.counter);
				ImGui::NewLine();
				ImGui::SliderFloat("", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);
				ImGui::Spacing();
				ImGui::Text("wear");



			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
			{
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("   CT Player Model");
				ImGui::PushItemWidth(160.f);
				ImGui::Combo("##TPlayerModel", &g_Options.playerModelCT, "Default\0Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman\0Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman\0Lieutenant Rex Krikey | SEAL Frogman\0Michael Syfers | FBI Sniper\0Operator | FBI SWAT\0Special Agent Ava | FBI\0Markus Delrow | FBI HRT\0Sous-Lieutenant Medic | Gendarmerie Nationale\0Chem-Haz Capitaine | Gendarmerie Nationale\0Chef d'Escadron Rouchard | Gendarmerie Nationale\0Aspirant | Gendarmerie Nationale\0Officer Jacques Beltram | Gendarmerie Nationale\0D Squadron Officer | NZSAS\0B Squadron Officer | SAS\0Seal Team 6 Soldier | NSWC SEAL\0Buckshot | NSWC SEAL\0Lt. Commander Ricksaw | NSWC SEAL\0'Blueberries' Buckshot | NSWC SEAL\0third Commando Company | KSK\0'Two Times' McCoy | TACP Cavalry\0'Two Times' McCoy | USAF TACP\0Primeiro Tenente | Brazilian 1st Battalion\0Cmdr. Mae 'Dead Cold' Jamison | SWAT\0first Lieutenant Farlow | SWAT\0John 'Van Healen' Kask | SWAT\0Bio-Haz Specialist | SWAT\0Sergeant Bombson | SWAT\0Chem-Haz Specialist | SWAT\0Getaway Sally | The Professionals\0Number K | The Professionals\0Little Kev | The Professionals\0Safecracker Voltzmann | The Professionals\0Bloody Darryl The Strapped | The Professionals\0Sir Bloody Loudmouth Darryl | The Professionals\0Sir Bloody Darryl Royale | The Professionals\0Sir Bloody Skullhead Darryl | The Professionals\0Sir Bloody Silent Darryl | The Professionals\0Sir Bloody Miami Darryl | The Professionals\0Street Soldier | Phoenix\0Soldier | Phoenix\0Slingshot | Phoenix\0Enforcer | Phoenix\0Mr. Muhlik | Elite Crew\0Prof. Shahmat | Elite Crew\0Osiris | Elite Crew\0Ground Rebel | Elite Crew\0The Elite Mr. Muhlik | Elite Crew\0Trapper | Guerrilla Warfare\0Trapper Aggressor | Guerrilla Warfare\0Vypa Sista of the Revolution | Guerrilla Warfare\0Col. Mangos Dabisi | Guerrilla Warfare\0Arno The Overgrown | Guerrilla Warfare\0'Medium Rare' Crasswater | Guerrilla Warfare\0Crasswater The Forgotten | Guerrilla Warfare\0Elite Trapper Solman | Guerrilla Warfare\0'The Doctor' Romanov | Sabre\0Blackwolf | Sabre\0Maximus | Sabre\0Dragomir | Sabre\0Rezan The Ready | Sabre\0Rezan the Redshirt | Sabre\0Dragomir | Sabre Footsoldier\0");

				ImGui::Text("   T Player Model");
				ImGui::PushItemWidth(160.f);
				ImGui::Combo("##CTPlayerModel", &g_Options.playerModelT, "Default\0Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman\0Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman\0Lieutenant Rex Krikey | SEAL Frogman\0Michael Syfers | FBI Sniper\0Operator | FBI SWAT\0Special Agent Ava | FBI\0Markus Delrow | FBI HRT\0Sous-Lieutenant Medic | Gendarmerie Nationale\0Chem-Haz Capitaine | Gendarmerie Nationale\0Chef d'Escadron Rouchard | Gendarmerie Nationale\0Aspirant | Gendarmerie Nationale\0Officer Jacques Beltram | Gendarmerie Nationale\0D Squadron Officer | NZSAS\0B Squadron Officer | SAS\0Seal Team 6 Soldier | NSWC SEAL\0Buckshot | NSWC SEAL\0Lt. Commander Ricksaw | NSWC SEAL\0'Blueberries' Buckshot | NSWC SEAL\0third Commando Company | KSK\0'Two Times' McCoy | TACP Cavalry\0'Two Times' McCoy | USAF TACP\0Primeiro Tenente | Brazilian 1st Battalion\0Cmdr. Mae 'Dead Cold' Jamison | SWAT\0first Lieutenant Farlow | SWAT\0John 'Van Healen' Kask | SWAT\0Bio-Haz Specialist | SWAT\0Sergeant Bombson | SWAT\0Chem-Haz Specialist | SWAT\0Getaway Sally | The Professionals\0Number K | The Professionals\0Little Kev | The Professionals\0Safecracker Voltzmann | The Professionals\0Bloody Darryl The Strapped | The Professionals\0Sir Bloody Loudmouth Darryl | The Professionals\0Sir Bloody Darryl Royale | The Professionals\0Sir Bloody Skullhead Darryl | The Professionals\0Sir Bloody Silent Darryl | The Professionals\0Sir Bloody Miami Darryl | The Professionals\0Street Soldier | Phoenix\0Soldier | Phoenix\0Slingshot | Phoenix\0Enforcer | Phoenix\0Mr. Muhlik | Elite Crew\0Prof. Shahmat | Elite Crew\0Osiris | Elite Crew\0Ground Rebel | Elite Crew\0The Elite Mr. Muhlik | Elite Crew\0Trapper | Guerrilla Warfare\0Trapper Aggressor | Guerrilla Warfare\0Vypa Sista of the Revolution | Guerrilla Warfare\0Col. Mangos Dabisi | Guerrilla Warfare\0Arno The Overgrown | Guerrilla Warfare\0'Medium Rare' Crasswater | Guerrilla Warfare\0Crasswater The Forgotten | Guerrilla Warfare\0Elite Trapper Solman | Guerrilla Warfare\0'The Doctor' Romanov | Sabre\0Blackwolf | Sabre\0Maximus | Sabre\0Dragomir | Sabre\0Rezan The Ready | Sabre\0Rezan the Redshirt | Sabre\0Dragomir | Sabre Footsoldier\0");

				if (ImGui::Button(" Update skins"))
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
				ImGui::NewLine();
				ImGui::NewLine();
				ImGui::NewLine();
				ImGui::Text(" Made with love by\n moonX <3");
				//static float rainbow; rainbow += 0.005f; if (rainbow > 1.f) rainbow = 0.f;
				//ImGui::TextColored(ImColor(rainbow, 1.f, 1.f), " https://t.me/resilth/");

			}
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

void Menu::UpdateStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_SliderGrabActive] = ImVec4(((float)g_Options.menu_color.r() / 255), ((float)g_Options.menu_color.g() / 255), ((float)g_Options.menu_color.b() / 255), 1.00f);
}

void Menu::Toggle()
{
	_visible = !_visible;
}

void Menu::CreateStyle()
{
	/*	ImGui::StyleColorsDark();
		ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
		_style.FrameRounding = 0.f;
		_style.WindowRounding = 0.f;
		_style.ChildRounding = 0.f;
		_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
		_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
		_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
		//_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.000f, 0.545f, 1.000f, 1.000f);
		//_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.060f, 0.416f, 0.980f, 1.000f);
		_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
		_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
		_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
		ImGui::GetStyle() = _style;*/

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(RGBA_TO_FLOAT(230, 230, 230, 255)));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(RGBA_TO_FLOAT(153, 153, 153, 255)));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(RGBA_TO_FLOAT(0, 0, 0, 102)));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(RGBA_TO_FLOAT(0, 0, 0, 0)));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(RGBA_TO_FLOAT(25, 25, 25, 235)));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(RGBA_TO_FLOAT(128, 128, 128, 128)));
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(RGBA_TO_FLOAT(0, 0, 0, 0)));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(RGBA_TO_FLOAT(110, 110, 110, 100)));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(RGBA_TO_FLOAT(120, 120, 120, 102)));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(RGBA_TO_FLOAT(102, 102, 102, 176)));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(RGBA_TO_FLOAT(69, 69, 138, 212)));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(RGBA_TO_FLOAT(82, 82, 161, 222)));
	ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(RGBA_TO_FLOAT(102, 102, 204, 51)));
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(RGBA_TO_FLOAT(102, 102, 141, 204)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(RGBA_TO_FLOAT(51, 64, 77, 0)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(RGBA_TO_FLOAT(102, 102, 204, 0)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(RGBA_TO_FLOAT(102, 102, 204, 0)));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(RGBA_TO_FLOAT(105, 100, 204, 0)));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(RGBA_TO_FLOAT(230, 230, 230, 128)));
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(RGBA_TO_FLOAT(255, 255, 255, 77)));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(RGBA_TO_FLOAT(105, 100, 204, 153)));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(RGBA_TO_FLOAT(90, 102, 156, 0)));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(RGBA_TO_FLOAT(102, 123, 182, 0)));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(RGBA_TO_FLOAT(118, 138, 204, 0)));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(RGBA_TO_FLOAT(102, 102, 102, 115)));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(RGBA_TO_FLOAT(102, 102, 102, 204)));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(RGBA_TO_FLOAT(102, 102, 102, 204)));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(RGBA_TO_FLOAT(128, 128, 128, 153)));
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(RGBA_TO_FLOAT(153, 153, 179, 255)));
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(RGBA_TO_FLOAT(179, 179, 230, 255)));
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(RGBA_TO_FLOAT(255, 255, 255, 41)));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(RGBA_TO_FLOAT(199, 210, 255, 153)));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(RGBA_TO_FLOAT(199, 210, 255, 230)));
	ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(RGBA_TO_FLOAT(0, 0, 255, 90)));
	ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(RGBA_TO_FLOAT(255, 255, 0, 230)));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.000000f,6.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.000000f,3.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.000000f,4.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 4.000000f,4.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 21.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.000000f,0.500000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.500000f,0.500000f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.500000f,0.500000f });
}

