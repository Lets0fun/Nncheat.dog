// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "../../Configuration/Config.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\visuals\hitchams.h"

using DoPostScreenEffects_t = void(__thiscall*)(void*, CViewSetup*);

void __fastcall hooks::hooked_postscreeneffects(void* thisptr, void* edx, CViewSetup* setup)
{
	auto cfg = c_config::get();
	static auto original_fn = clientmode_hook->get_func_address <DoPostScreenEffects_t>(44);

	if (!cfg->auto_check(cfg->i["esp_en"], cfg->i["esp_en_type"]))
		return original_fn(thisptr, setup);

	if (!g_ctx.local())
		return original_fn(thisptr, setup);

	hit_chams::get().draw_hit_matrix();

	for (auto i = 0; i < m_glow()->m_GlowObjectDefinitions.Count(); i++) //-V807
	{
		if (m_glow()->m_GlowObjectDefinitions[i].IsUnused())
			continue;

		auto object = &m_glow()->m_GlowObjectDefinitions[i];
		auto entity = object->GetEnt();

		if (!entity)
			continue;

		auto client_class = entity->GetClientClass();

		if (!client_class)
			continue;

		if (entity->is_player())
		{
			auto e = (player_t*)entity;
			auto should_glow = false;

			if (!e->valid(false, false))
				continue;

			auto type = ENEMY;

			if (e == g_ctx.local())
				type = LOCAL;
			else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
				type = TEAM;

			float color[4];

			if (cfg->c_config::get()->b["glow"] && type == ENEMY)
			{
				should_glow = true;

				color[0] = cfg->c["glowcoll"][0] / 255.0f;
				color[1] = cfg->c["glowcoll"][1] / 255.0f;
				color[2] = cfg->c["glowcoll"][2] / 255.0f;
				color[3] = cfg->c["glowcoll"][3] / 255.0f;
			}
			else if (cfg->c_config::get()->b["glow"] && cfg->b["teammates"] && type == TEAM)
			{
				should_glow = true;

				color[0] = cfg->c["glowcoll"][0] / 255.0f;
				color[1] = cfg->c["glowcoll"][1] / 255.0f;
				color[2] = cfg->c["glowcoll"][2] / 255.0f;
				color[3] = cfg->c["glowcoll"][3] / 255.0f;
			}

			if (!should_glow)
				continue;

			object->Set
			(
				color[0],
				color[1],
				color[2],
				c_config::get()->b["hide_from_obs"] && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
				1.0f,
				0
			);
		}
		else if (cfg->m["drpsweapons"][2] && (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor)) //-V648
		{
			float color[4] =
			{
				color[0] = cfg->c["dropwepcoll"][0] / 255.0f,
				color[1] = cfg->c["dropwepcoll"][1] / 255.0f,
				color[2] = cfg->c["dropwepcoll"][2] / 255.0f,
				color[3] = cfg->c["dropwepcoll"][3] / 255.0f
			};

			object->Set
			(
				color[0],
				color[1],
				color[2],
				c_config::get()->b["hide_from_obs"] && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
				1.0f,
				0
			);
		}
		else if (cfg->b["grenades"])
		{
			auto model = entity->GetModel();

			if (!model)
				continue;

			auto studio_model = m_modelinfo()->GetStudioModel(model);

			if (!studio_model)
				continue;

			std::string name = studio_model->szName;

			if (name.find("flashbang") == std::string::npos && name.find("smokegrenade") == std::string::npos && name.find("incendiarygrenade") == std::string::npos && name.find("molotov") == std::string::npos && name.find("fraggrenade") == std::string::npos && name.find("decoy") == std::string::npos)
				continue;

			if (name.find("thrown") != std::string::npos ||
				client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CDecoyProjectile || client_class->m_ClassID == CMolotovProjectile)
			{
				float color[4] =
				{
					color[0] = cfg->c["grencoll"][0] / 255.0f,
					color[1] = cfg->c["grencoll"][1] / 255.0f,
					color[2] = cfg->c["grencoll"][2] / 255.0f,
					color[3] = cfg->c["grencoll"][3] / 255.0f
				};

				object->Set
				(
					color[0],
					color[1],
					color[2],
					c_config::get()->b["hide_from_obs"] && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
					1.0f,
					0
				);
			}
			else if (name.find("dropped") != std::string::npos && cfg->m["drpsweapons"][2])
			{
				float color[4] =
				{
				color[0] = cfg->c["dropwepcoll"][0] / 255.0f,
				color[1] = cfg->c["dropwepcoll"][1] / 255.0f,
				color[2] = cfg->c["dropwepcoll"][2] / 255.0f,
				color[3] = cfg->c["dropwepcoll"][3] / 255.0f
				};

				object->Set
				(
					color[0],
					color[1],
					color[2],
					c_config::get()->b["hide_from_obs"] && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
					1.0f,
					0
				);
			}

		}
	}

	original_fn(thisptr, setup);
}