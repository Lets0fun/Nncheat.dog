// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\lagcompensation\local_animations.h"
#include "..\..\cheats\visuals\player_esp.h"
// hitchams ../../cheats/visual/hitchams.h
#include "../../cheats/visuals/hitchams.h"
#include "../Configuration/Config.h"

IMaterial* CreateMaterial(bool lit, const std::string& material_data)
{
	static auto created = 0;
	std::string type = lit ? crypt_str("VertexLitGeneric") : crypt_str("UnlitGeneric");

	auto matname = crypt_str("gsfun_") + std::to_string(created);
	++created;

	auto keyValues = new KeyValues(matname.c_str());
	static auto key_values_address = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 56 8B F1 33 C0 8B 4D 0C 81 26 ? ? ? ? 89 46 10 8A 45 10 C6 46 03 00 89 4E 14 88 46 18 C7 46 ? ? ? ? ? C7 46"));

	using KeyValuesFn = void(__thiscall*)(void*, const char*, void*, void*);
	reinterpret_cast <KeyValuesFn> (key_values_address)(keyValues, type.c_str(), 0, 0);

	static auto load_from_buffer_address = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89"));
	using LoadFromBufferFn = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*);

	reinterpret_cast <LoadFromBufferFn> (load_from_buffer_address)(keyValues, matname.c_str(), material_data.c_str(), nullptr, nullptr, nullptr);

	auto material = m_materialsystem()->CreateMaterial(matname.c_str(), keyValues);
	material->IncrementReferenceCount();

	return material;
}

using DrawModelExecute_t = void(__thiscall*)(IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);

void __stdcall hooks::hooked_dme(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone_to_world)
{
	static auto original_fn = modelrender_hook->get_func_address <DrawModelExecute_t>(21);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	auto cfg = c_config::get();
	if (!c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	//if (g_cfg.player_list.disablevisuals[player])

	if (m_engine()->IsTakingScreenshot() && c_config::get()->b["hide_from_obs"])
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	if (!info.pModel)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	auto model_entity = static_cast<player_t*>(m_entitylist()->GetClientEntity(info.entity_index));
	auto name = m_modelinfo()->GetModelName(info.pModel);

	auto is_player = strstr(name, "models/player") && model_entity->is_alive() && (cfg->b["player"] || cfg->b["teammate"] || cfg->b["localplayer"] || c_config::get()->b["localplayerfake"]);
	auto is_weapon = strstr(name, "weapons/v_") && !strstr(name, "arms") && cfg->b["wepvm"];
	auto is_arms = strstr(name, "arms") && c_config::get()->b["hands"];
	auto is_sleeve = strstr(name, "sleeve") && c_config::get()->b["hands"];
	auto weapon_on_back = strstr(name, "_dropped.mdl") && strstr(name, "models/weapons/w") && !strstr(name, "arms") && !strstr(name, "ied_dropped") && cfg->b["wepvm"];
	auto weapon_enemy_hands = strstr(name, "models/weapons/w") && !strstr(name, "arms") && !strstr(name, "ied_dropped") && cfg->b["wepvm"];

	if (m_modelrender()->IsForcedMaterialOverride() && !is_weapon && !is_arms && !is_sleeve && !weapon_on_back && !weapon_enemy_hands)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f); //-V807

	if (!is_player && !is_weapon && !is_arms && !is_sleeve && !weapon_on_back && !weapon_enemy_hands)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	static IMaterial* materials[] =
	{
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"0"
			}
		)#")), // default
		CreateMaterial(false, crypt_str(R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#")), //solid (flat)

		m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/dogtags/dogtags_outline"), "Model texture"), //blikani, flat i guess

		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
							"$basetexture" "vgui/white_additive"
				  "$ignorez" "0"
				  "$envmap" "env_cubemap"
				  "$normalmapalphaenvmapmask" "1"
				  "$envmapcontrast"  "1"
				  "$nofog" "1"
				  "$model" "1"
				  "$nocull" "0"
				  "$selfillum" "1"
				  "$halflambert" "1"
				  "$znearer" "0"
				  "$flat" "1"
			}
		)#")),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
            {
                "$additive" "0.5"
                "$envmap" "models/effects/cube_white"
                "$ignorez"                    "1"
                "$envmaptint" "[1 1 1]"
                "$envmapfresnel" "1"
                "$envmapfresnelminmaxexp" "[0 1 2]"
                "$alpha" "1"
                "$wireframe"                "0"
            }
        )#")), // glow
		m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/crystal_clear"), nullptr), // divne
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				  "$basetexture" "vgui/white_additive"
				  "$nofog" "1"
				  "$model" "1"
				  "$flat"  "0"
			}
		)#")), //metallic
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"1"
			}
		)#")),  //wireframe


		//zbytek se neukazuje

		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric" 
			{ 
				"$additive"					"1" 
				"$envmap"					"models/effects/cube_white" 
				"$envmaptint"				"[1 1 1]" 
				"$envmapfresnel"			"1" 
				"$envmapfresnelminmaxexp" 	"[0 1 2]" 
				"$alpha" 					"0.7" 
			}
		)#")), // glow
		/*
		m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/cologne_prediction/cologne_prediction_glass"), nullptr), // divne sklo
		m_materialsystem()->FindMaterial(crypt_str("dev/glow_armsrace.vmt"), nullptr), // outline glow

		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"dev/zone_warning"
				"$additive"					"1"
				"$envmap"					"editor/cube_vertigo"
				"$envmaptint"				"[0 0.5 0.55]"
				"$envmapfresnel"			"1"
				"$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
				"$alpha"					"1"

				Proxies
				{
					TextureScroll
					{
						"texturescrollvar"			"$baseTextureTransform"
						"texturescrollrate"			"0.25"
						"texturescrollangle"		"270"
					}
					Sine
					{
						"sineperiod"				"2"
						"sinemin"					"0.1"
						"resultVar"					"$envmapfresnelminmaxexp[1]"
					}
				}
			}
		)#")),
		*/

	};

	auto called_original = false;


	if (is_player)
	{
		auto type = ENEMY;

		if (model_entity == g_ctx.local())
			type = LOCAL;
		else if (model_entity->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
			type = TEAM;

		if (type == ENEMY)
		{
			auto alpha_modifier = playeresp::get().esp_alpha_fade[model_entity->EntIndex()];

			auto material = materials[cfg->i["plyr_xqzzz"]];
			auto double_material = materials[7]; //materials[7];;

			if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				if (cfg->b["player"] && cfg->b["player_xqz"])
				{
					auto alpha = (float)g_cfg.player.backtrack_chams_color.a() / 255.0f;;

					alpha = (float)cfg->c["xqzcoll"][3] / 255.0f;

					float xqz_color[3] =
					{
						cfg->c["xqzcoll"][0] / 255.0f,
						cfg->c["xqzcoll"][1] / 255.0f,
						cfg->c["xqzcoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha * alpha_modifier); //-V807
					util::color_modulate(xqz_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					alpha = (float)cfg->c["playercoll"][3] / 255.0f;

					float normal_color[3] =
					{
						 cfg->c["playercoll"][0] / 255.0f,
						 cfg->c["playercoll"][1] / 255.0f,
						 cfg->c["playercoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha * alpha_modifier);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (cfg->i["plyr_xqzzz"] == 4)
					{
						alpha = (float)cfg->c["enemydoublecolor"][3] / 255.0f;

						float double_color[3] =
						{
							cfg->c["enemydoublecolor"][0] / 255.0f,
							cfg->c["enemydoublecolor"][1] / 255.0f,
							cfg->c["enemydoublecolor"][2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, g_ctx.globals.fake_matrix);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
				else if (cfg->b["player"])
				{
					auto alpha = (float)g_cfg.player.backtrack_chams_color.a() / 255.0f;;

					alpha = (float)cfg->c["playercoll"][3] / 255.0f;

					float normal_color[3] =
					{
						 cfg->c["playercoll"][0] / 255.0f,
						 cfg->c["playercoll"][1] / 255.0f,
						 cfg->c["playercoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha * alpha_modifier);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (cfg->i["plyr_xqzzz"] == 4)
					{
						alpha = (float)cfg->c["enemydoublecolor"][3] / 255.0f;

						float double_color[3] =
						{
							cfg->c["enemydoublecolor"][0] / 255.0f,
							cfg->c["enemydoublecolor"][1] / 255.0f,
							cfg->c["enemydoublecolor"][2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, g_ctx.globals.fake_matrix);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
			}

			if (!called_original)
				return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
		}
		else if (type == TEAM)
		{
			auto material = materials[cfg->i["teammate_xqzzz"]];
			auto double_material = materials[8]; //materials[7];

			if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				if (cfg->b["teammate"] && cfg->b["teammate_xqz"])
				{
					auto alpha = (float)cfg->c["txqzcoll"][3] / 255.0f;

					float xqz_color[3] =
					{
						cfg->c["txqzcoll"][0] / 255.0f,
						cfg->c["txqzcoll"][1] / 255.0f,
						cfg->c["txqzcoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(xqz_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					alpha = (float)cfg->c["teamcoll"][3] / 255.0f;

					float normal_color[3] =
					{
						cfg->c["teamcoll"][0] / 255.0f,
						cfg->c["teamcoll"][1] / 255.0f,
						cfg->c["teamcoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);



					called_original = true;
				}
				else if (cfg->b["teammate"])
				{
					auto alpha = (float)cfg->c["teamcoll"][3] / 255.0f;

					float normal_color[3] =
					{
						cfg->c["teamcoll"][0] / 255.0f,
						cfg->c["teamcoll"][1] / 255.0f,
						cfg->c["teamcoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);



					called_original = true;
				}
			}

			if (!called_original)
				return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
		}
		else if (m_input()->camera_third_person)
		{
			auto alpha_modifier = 1.0f;

			if (c_config::get()->m["lptrans"][1] && g_ctx.globals.scoped)
				alpha_modifier = 0.5f;

			else if (c_config::get()->m["lptrans"][0] && g_ctx.globals.grenade)
				alpha_modifier = 0.5f;

			auto material = materials[cfg->i["localmaterial"]];
			auto double_material = materials[8]; //materials[7];

			if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				if (cfg->b["localplayer"])
				{
					auto alpha = (float)cfg->c["lpcoll"][3] / 255.0f * alpha_modifier;

					float xqz_color[3] =
					{
						cfg->c["lpcoll"][0] / 255.0f,
						cfg->c["lpcoll"][1] / 255.0f,
						cfg->c["lpcoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(xqz_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					alpha = (float)cfg->c["lpcoll"][3] / 255.0f * alpha_modifier;

					float normal_color[3] =
					{
						cfg->c["lpcoll"][0] / 255.0f,
						cfg->c["lpcoll"][1] / 255.0f,
						cfg->c["lpcoll"][2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (cfg->i["localmaterial"] == 4)
					{
						alpha = (float)cfg->c["localdoublecolor"][3] / 255.0f;

						float double_color[3] =
						{
							cfg->c["localdoublecolor"][0] / 255.0f,
							cfg->c["localdoublecolor"][1] / 255.0f,
							cfg->c["localdoublecolor"][2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, g_ctx.globals.fake_matrix);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
			}

			if (!called_original && cfg->b["disoccu"])
			{
				m_renderview()->SetBlend(alpha_modifier);
				m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			}

			if (c_config::get()->b["localplayerfake"])
			{
				//if (!local_animations::get().local_data.visualize_lag)
				//{
				//	for (auto& i : g_ctx.globals.fake_matrix)
				//	{
				//		i[0][3] += info.origin.x;
				//		i[1][3] += info.origin.y;
				//		i[2][3] += info.origin.z;
				//	}
				//}

				auto alpha = (float)c_config::get()->c["lpfcoll"][3] / 255.0f;
				material = materials[g_cfg.player.fake_chams_type];

				float fake_color[3] =
				{
					c_config::get()->c["lpfcoll"][0] / 255.0f,
					c_config::get()->c["lpfcoll"][1] / 255.0f,
					c_config::get()->c["lpfcoll"][2] / 255.0f
				};

				m_renderview()->SetBlend(alpha);
				util::color_modulate(fake_color, material);

				material->IncrementReferenceCount();
				material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				m_modelrender()->ForcedMaterialOverride(material);
				original_fn(m_modelrender(), ctx, state, info, g_ctx.globals.fake_matrix);
				m_modelrender()->ForcedMaterialOverride(nullptr);


				//if (!local_animations::get().local_data.visualize_lag)
				//{
				//	for (auto& i : g_ctx.globals.fake_matrix)
				//	{
				//		i[0][3] -= info.origin.x;
				//		i[1][3] -= info.origin.y;
				//		i[2][3] -= info.origin.z;
				//	}
				//}
			}

			if (!called_original && !g_cfg.player.layered)
			{
				m_renderview()->SetBlend(alpha_modifier);
				m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			}
		}
	}
	else if (is_weapon)
	{
		auto alpha = c_config::get()->c["weppvmcoll"][3] / 255.0f;

		auto material = materials[c_config::get()->i["weaponchams"]];
		auto double_material = materials[6];

		if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
		{
			float weapon_color[3] =
			{
				c_config::get()->c["weppvmcoll"][0] / 255.0f,
				c_config::get()->c["weppvmcoll"][1] / 255.0f,
				c_config::get()->c["weppvmcoll"][2] / 255.0f
			};

			m_renderview()->SetBlend(alpha);
			util::color_modulate(weapon_color, material);

			material->IncrementReferenceCount();
			material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			m_modelrender()->ForcedMaterialOverride(material);
			original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			m_modelrender()->ForcedMaterialOverride(nullptr);

			called_original = true;
		}

		if (!called_original)
			return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
	}
	else if (is_arms || is_sleeve)
	{
		auto alpha = (float)cfg->c["handscoll"][3] / 255.0f;

		auto material = materials[cfg->i["handchams"]];
		auto double_material = materials[8];//materials[7];

		if (material && cfg->b["hands"] && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
		{
			float arms_color[3] =
			{
				cfg->c["handscoll"][0] / 255.0f,
				cfg->c["handscoll"][1] / 255.0f,
				cfg->c["handscoll"][2] / 255.0f
			};

			m_renderview()->SetBlend(alpha);
			util::color_modulate(arms_color, material);

			material->IncrementReferenceCount();
			material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			m_modelrender()->ForcedMaterialOverride(material);
			original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			m_modelrender()->ForcedMaterialOverride(nullptr);

			if (cfg->i["handdoublecolor"] == 4)
			{
				alpha = (float)cfg->c["handdoublecolor"][3] / 255.0f;

				float double_color[3] =
				{
					cfg->c["handdoublecolor"][0] / 255.0f,
					cfg->c["handdoublecolor"][1] / 255.0f,
					cfg->c["handdoublecolor"][2] / 255.0f
				};

				m_renderview()->SetBlend(alpha);
				util::color_modulate(double_color, double_material);

				double_material->IncrementReferenceCount();
				double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				m_modelrender()->ForcedMaterialOverride(double_material);
				original_fn(m_modelrender(), ctx, state, info, g_ctx.globals.fake_matrix);
				m_modelrender()->ForcedMaterialOverride(nullptr);
			}

			called_original = true;
		}

		if (!called_original)
			return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
	}
	else if (weapon_on_back || weapon_enemy_hands)
	{
		auto alpha = (float)c_config::get()->c["weppvmcoll"][3] / 255.0f;

		auto material = materials[c_config::get()->i["weaponchams"]];
		auto double_material = materials[6];

		if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
		{
			float attachment_color[3] =
			{
				c_config::get()->c["weppvmcoll"][0] / 255.0f,
				c_config::get()->c["weppvmcoll"][1] / 255.0f,
				c_config::get()->c["weppvmcoll"][2] / 255.0f
			};

			m_renderview()->SetBlend(alpha);
			util::color_modulate(attachment_color, material);

			material->IncrementReferenceCount();
			material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			m_modelrender()->ForcedMaterialOverride(material);
			original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			m_modelrender()->ForcedMaterialOverride(nullptr);

			called_original = true;
		}

		if (!called_original)
			return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
	}
}

void hit_chams::draw_hit_matrix()
{
	if (!g_ctx.available())
		m_Hitmatrix.clear();

	if (m_Hitmatrix.empty())
		return;

	if (!m_modelrender())
		return;

	auto ctx = m_materialsystem()->GetRenderContext();

	if (!ctx)
		return;

	auto it = m_Hitmatrix.begin();
	while (it != m_Hitmatrix.end()) {
		if (!it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			!it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel) {
			++it;
			continue;
		}

		auto alpha = 1.0f;
		auto delta = m_globals()->m_realtime - it->time;
		if (delta > 0.0f) {
			alpha -= delta;
			if (delta > 1.0f) {
				it = m_Hitmatrix.erase(it);
				continue;
			}
		}

		static IMaterial* materials[] =
		{
			CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"0"
			}
		)#")),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture" "vgui/white_additive"
				"$ignorez" "0"
				"$envmap" "env_cubemap"
				"$normalmapalphaenvmapmask" "1"
				"$envmapcontrast"  "1"
				"$nofog" "1"
				"$model" "1"
				"$nocull" "0"
				"$selfillum" "1"
				"$halflambert" "1"
				"$znearer" "0"
				"$flat" "1"
			}
		)#")),
		CreateMaterial(false, crypt_str(R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#")),
		m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/dogtags/dogtags_outline"), "Model texture"),
		m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/crystal_clear"), nullptr),
		m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/cologne_prediction/cologne_prediction_glass"), nullptr),
		m_materialsystem()->FindMaterial(crypt_str("dev/glow_armsrace.vmt"), nullptr),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric" 
			{ 
				"$additive"					"1" 
				"$envmap"					"models/effects/cube_white" 
				"$envmaptint"				"[1 1 1]" 
				"$envmapfresnel"			"1" 
				"$envmapfresnelminmaxexp" 	"[0 1 2]" 
				"$alpha" 					"0.7" 
			}
		)#")),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
		    {
		        "$basetexture"				"dev/zone_warning"
		        "$additive"					"1"
		        "$envmap"					"editor/cube_vertigo"
		        "$envmaptint"				"[0 0.5 0.55]"
		        "$envmapfresnel"			"1"
		        "$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
		        "$alpha"					"0"
   
		        Proxies
		        {
		            TextureScroll
		            {
		                "texturescrollvar"			"$baseTextureTransform"
		                "texturescrollrate"			"0.25"
		                "texturescrollangle"		"270"
		            }
		            Sine
		            {
		                "sineperiod"				"2"
		                "sinemin"					"0.1"
		                "resultVar"					"$envmapfresnelminmaxexp[1]"
		            }
		        }
		    }
		)#")),
		m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/velvet"), nullptr),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
		    {
				 "$basetexture" "dev/dev_camo"
				 "$envmap" "dev/dev_camoenvmap"

				 "Proxies"
				 {
          				 "Camo"
            		     {
				 "camopatterntexture" "hl2/materials/dev/dev_camo.tga"
               	 "camoboundingboxmin" "[ 0.00 0.00 0.00 ]"
               	 "camoboundingboxmax" "[ 1.00 1.00 1.00 ]"
              	 "surfaceprop" "concrete"
				 }
			 }
		}
	)#"))
		};

		auto material = materials[g_cfg.player.lag_hitbox_type];

		auto alpha_c = (float)g_cfg.player.lag_hitbox_color.a() / 255.0f;
		float normal_color[3] =
		{
			g_cfg.player.lag_hitbox_color[0] / 255.0f,
			g_cfg.player.lag_hitbox_color[1] / 255.0f,
			g_cfg.player.lag_hitbox_color[2] / 255.0f
		};

		m_renderview()->SetBlend(alpha_c * alpha);
		util::color_modulate(normal_color, material);

		material->IncrementReferenceCount();
		material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

		m_modelrender()->ForcedMaterialOverride(material);
		m_modelrender()->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);
		m_modelrender()->ForcedMaterialOverride(nullptr);

		///////////////////////////////////////////////////////////////////////////////////

		auto asd_alpha = (float)g_cfg.player.type[ENEMY].xqz_color.a() / 255.0f;

		float xqz_color[3] =
		{
			g_cfg.player.lag_hitbox_color[0] / 255.0f,
			g_cfg.player.lag_hitbox_color[1] / 255.0f,
			g_cfg.player.lag_hitbox_color[2] / 255.0f
		};

		m_renderview()->SetBlend(asd_alpha * alpha_c);
		util::color_modulate(xqz_color, material);

		material->IncrementReferenceCount();
		material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

		m_modelrender()->ForcedMaterialOverride(material);
		m_modelrender()->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);
		m_modelrender()->ForcedMaterialOverride(nullptr);

		++it;
	}
}