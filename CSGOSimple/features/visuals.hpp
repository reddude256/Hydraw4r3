#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"


struct MovementData_t
{
	MovementData_t() = default;
	MovementData_t(float _flVelocity, bool _bOnGround) {
		flvelocity = _flVelocity;
		bOnGround = _bOnGround;
	}

	float flvelocity;
	bool bOnGround;
};

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;

		bool Begin(C_BasePlayer * pl);
		void RenderBox(C_BaseEntity* pl);
		void RenderName(C_BaseEntity* pl);
		void RenderWeaponName(C_BaseEntity* pl);
		void RenderHealth(C_BaseEntity* pl);
		void RenderArmour();
		void RenderSnapline();
	};
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void ThirdPerson();
	void ebdetection(float unpred_z, int unpred_flags);
	void GatherMovementData();
	void PaintMovementData();
	std::vector<MovementData_t> vecMovementData;
public:
	void AddToDrawList();
	void Render();
	void acpect(float numbero)
	{
		ConVar* yessoftware = g_CVar->FindVar("r_aspectratio");
		*(int*)((DWORD)&yessoftware->m_fnChangeCallbacks + 0xC) = 0;
		yessoftware->SetValue(numbero);
	}

};
