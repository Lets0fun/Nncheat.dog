// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "misc.h"
#include "fakelag.h"
#include "..\ragebot\aim.h"
#include "..\visuals\world_esp.h"
#include "prediction_system.h"
#include "logs.h"
#include "..\visuals\hitchams.h"
#include "../menu_alpha.h"
#include "../Configuration/Config.h"
#include <Menu/MenuFramework/Renderer.h>

#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

void misc::EnableHiddenCVars()
{
	auto p = **reinterpret_cast<ConCommandBase***>(reinterpret_cast<DWORD>(m_cvar()) + 0x34);

	for (auto c = p->m_pNext; c != nullptr; c = c->m_pNext) {
		c->m_nFlags &= ~FCVAR_DEVELOPMENTONLY; // FCVAR_DEVELOPMENTONLY
		c->m_nFlags &= ~FCVAR_HIDDEN; // FCVAR_HIDDEN
	}
}

void misc::pingspike()
{
	int value = c_config::get()->i["misc_pingspike_val"] / 2;
	ConVar* net_fakelag = m_cvar()->FindVar(crypt_str("net_fakelag"));
	if (c_config::get()->b["misc_ping_spike"] && c_config::get()->auto_check(c_config::get()->i["ping_spike_key"], c_config::get()->i["ping_spike_key_style"]))
		net_fakelag->SetValue(value);
	else
	{
		net_fakelag->SetValue(0);
	}


}

void misc::NoDuck(CUserCmd* cmd)
{
	if (!c_config::get()->b["inf_duck"])
		return;

	if (m_gamerules()->m_bIsValveDS())
		return;

	cmd->m_buttons |= IN_BULLRUSH;
}

void misc::AutoCrouch(CUserCmd* cmd)
{
	if (fakelag::get().condition)
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (m_gamerules()->m_bIsValveDS())
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (!c_config::get()->auto_check(c_config::get()->i["rage_fd_enabled"], c_config::get()->i["rage_fd_enabled_style"]))
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (!g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands != 7)
		return;

	if (m_clientstate()->iChokedCommands >= 7)
		cmd->m_buttons |= IN_DUCK;
	else
		cmd->m_buttons &= ~IN_DUCK;

	g_ctx.globals.fakeducking = true;
}

void misc::watermark()
{
	if (!c_config::get()->b["misc_watermark"])
		return;

	auto game = crypt_str("game           | ");
	auto sense = crypt_str("      sense");

	int fps = floor(1 / m_globals()->m_frametime);
	auto text = game + g_ctx.username + crypt_str(" | ") + std::to_string(fps) + crypt_str("fps | ") + g_ctx.globals.time;

	auto w = render::get().text_width(fonts[NAME], text.c_str()) + 8;
	auto h = 18;

	int width, height;
	m_engine()->GetScreenSize(width, height);

	auto x = width - 2;
	auto y = 12;
	x = x - w - 10;

	render::get().rect_filled(x - 6, y - 6, w + 13, h + 14, Color(0, 0, 0, 255));
	render::get().rect_filled(x - 5, y - 5, w + 11, h + 12, Color(34, 34, 34, 255));
	render::get().rect_filled(x + 1, y, w, h + 1, Color(0, 0, 0, 255));

	render::get().rect(x - 1, y - 1, w + 3, h + 3, Color(56, 56, 56, 255));
	render::get().rect(x - 5, y - 5, w + 11, h + 12, Color(56, 56, 56, 255));

	render::get().gradient(x + 2, y + 1, (w / 2) - 1, 1, Color(59, 175, 222, 255), Color(202, 70, 205, 255), GRADIENT_HORIZONTAL);
	render::get().gradient(x + 1 + (w / 2), y + 1, (w / 2) - 1, 1, Color(202, 70, 205, 255), Color(201, 227, 58, 255), GRADIENT_HORIZONTAL);
	render::get().gradient(x + 2, y + 2, (w / 2) - 1, 1, Color(59, 175, 222, 130), Color(202, 70, 205, 130), GRADIENT_HORIZONTAL);
	render::get().gradient(x + 1 + (w / 2), y + 2, (w / 2) - 1, 1, Color(202, 70, 205, 130), Color(201, 227, 58, 130), GRADIENT_HORIZONTAL);

	render::get().text(fonts[ESP], x + 4, y + 12, Color(0, 0, 0, 180), HFONT_CENTERED_Y, text.c_str());
	render::get().text(fonts[ESP], x + 4, y + 11, Color(255, 255, 255, 255), HFONT_CENTERED_Y, text.c_str());
	render::get().text(fonts[ESP], x + 13, y + 12, Color(0, 0, 0, 180), HFONT_CENTERED_Y, sense);
	render::get().text(fonts[ESP], x + 13, y + 11, Color(160, 200, 80, 255), HFONT_CENTERED_Y, sense);
}

void misc::SlideWalk(CUserCmd* cmd)
{
	if (!g_ctx.local()->is_alive()) //-V807
		return;

	if (g_ctx.local()->get_move_type() == MOVETYPE_LADDER)
		return;

	if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	if (antiaim::get().condition(cmd, true) && (g_cfg.misc.legs_movement == 2 && math::random_int(0, 2) == 0 || g_cfg.misc.legs_movement == 1))
	{
		if (cmd->m_forwardmove > 0.0f)
		{
			cmd->m_buttons |= IN_BACK;
			cmd->m_buttons &= ~IN_FORWARD;
		}
		else if (cmd->m_forwardmove < 0.0f)
		{
			cmd->m_buttons |= IN_FORWARD;
			cmd->m_buttons &= ~IN_BACK;
		}

		if (cmd->m_sidemove > 0.0f)
		{
			cmd->m_buttons |= IN_MOVELEFT;
			cmd->m_buttons &= ~IN_MOVERIGHT;
		}
		else if (cmd->m_sidemove < 0.0f)
		{
			cmd->m_buttons |= IN_MOVERIGHT;
			cmd->m_buttons &= ~IN_MOVELEFT;
		}
	}
	else
	{
		auto buttons = cmd->m_buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

		if (c_config::get()->i["leg_movementtype"] == 1 && math::random_int(0, 1)) //|| c_config::get()->i["leg_movementtype"] == 1)
		{
			if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
				return;

			if (cmd->m_forwardmove <= 0.0f)
				buttons |= IN_BACK;
			else
				buttons |= IN_FORWARD;

			if (cmd->m_sidemove > 0.0f)
				goto LABEL_15;
			else if (cmd->m_sidemove >= 0.0f)
				goto LABEL_18;

			goto LABEL_17;
		}
		else
			goto LABEL_18;

		if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
			return;

		if (cmd->m_forwardmove <= 0.0f) //-V779
			buttons |= IN_FORWARD;
		else
			buttons |= IN_BACK;

		if (cmd->m_sidemove > 0.0f)
		{
		LABEL_17:
			buttons |= IN_MOVELEFT;
			goto LABEL_18;
		}

		if (cmd->m_sidemove < 0.0f)
			LABEL_15:

		buttons |= IN_MOVERIGHT;

	LABEL_18:
		cmd->m_buttons = buttons;
	}
}

void misc::automatic_peek(CUserCmd* cmd, float wish_yaw)
{
	if (!g_ctx.globals.weapon->is_non_aim() && c_config::get()->b["rage_quick_peek_assist"] && c_config::get()->auto_check(c_config::get()->i["rage_quickpeek_enabled"], c_config::get()->i["rage_quickpeek_enabled_style"]))
	{
		if (g_ctx.globals.start_position.IsZero())
		{
			g_ctx.globals.start_position = g_ctx.local()->GetAbsOrigin();

			if (!(engineprediction::get().backup_data.flags & FL_ONGROUND))
			{
				Ray_t ray;
				CTraceFilterWorldAndPropsOnly filter;
				CGameTrace trace;

				ray.Init(g_ctx.globals.start_position, g_ctx.globals.start_position - Vector(0.0f, 0.0f, 8000.0f));
				m_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);

				if (trace.fraction < 1.0f)
					g_ctx.globals.start_position = trace.endpos + Vector(0.0f, 0.0f, 3.0f);
			}
		}
		else
		{
			auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);

			if (cmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
				g_ctx.globals.fired_shot = true;

			if (g_ctx.globals.fired_shot)
			{
				auto current_position = g_ctx.local()->GetAbsOrigin();
				auto difference = current_position - g_ctx.globals.start_position;

				if (difference.Length2D() > 1.0f)
				{
					auto velocity = Vector(difference.x * cos(wish_yaw / 180.0f * M_PI) + difference.y * sin(wish_yaw / 180.0f * M_PI), difference.y * cos(wish_yaw / 180.0f * M_PI) - difference.x * sin(wish_yaw / 180.0f * M_PI), difference.z);

					cmd->m_forwardmove = -velocity.x * 20;
					cmd->m_sidemove = velocity.y * 20;
				}
				else
				{
					g_ctx.globals.fired_shot = false;
					g_ctx.globals.start_position.Zero();
				}
			}
		}
	}
	else
	{
		g_ctx.globals.fired_shot = false;
		g_ctx.globals.start_position.Zero();
	}
}

void misc::ViewModel()
{
	if (g_cfg.esp.viewmodel_fov)
	{
		auto viewFOV = (float)g_cfg.esp.viewmodel_fov + 68.0f;
		static auto viewFOVcvar = m_cvar()->FindVar(crypt_str("viewmodel_fov"));

		if (viewFOVcvar->GetFloat() != viewFOV) //-V550
		{
			*(float*)((DWORD)&viewFOVcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewFOVcvar->SetValue(viewFOV);
		}
	}

	if (g_cfg.esp.viewmodel_x)
	{
		auto viewX = (float)g_cfg.esp.viewmodel_x / 2.0f;
		static auto viewXcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_x")); //-V807

		if (viewXcvar->GetFloat() != viewX) //-V550
		{
			*(float*)((DWORD)&viewXcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewXcvar->SetValue(viewX);
		}
	}

	if (g_cfg.esp.viewmodel_y)
	{
		auto viewY = (float)g_cfg.esp.viewmodel_y / 2.0f;
		static auto viewYcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_y"));

		if (viewYcvar->GetFloat() != viewY) //-V550
		{
			*(float*)((DWORD)&viewYcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewYcvar->SetValue(viewY);
		}
	}

	if (g_cfg.esp.viewmodel_z)
	{
		auto viewZ = (float)g_cfg.esp.viewmodel_z / 2.0f;
		static auto viewZcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_z"));

		if (viewZcvar->GetFloat() != viewZ) //-V550
		{
			*(float*)((DWORD)&viewZcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewZcvar->SetValue(viewZ);
		}
	}
}

void misc::FullBright()
{
	if (!c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
		return;

	static auto mat_fullbright = m_cvar()->FindVar(crypt_str("mat_fullbright"));

	if (mat_fullbright->GetBool() != c_config::get()->m["brightadj"][0])
		mat_fullbright->SetValue(c_config::get()->m["brightadj"][0]);
}

void misc::DrawGray()
{
	if (!c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
		return;

	static auto mat_drawgray = m_cvar()->FindVar(crypt_str("mat_drawgray"));

	if (mat_drawgray->GetBool() != g_cfg.esp.drawgray)
		mat_drawgray->SetValue(g_cfg.esp.drawgray);
}

void misc::PovArrows(player_t* e, Color color)
{
	auto cfg = c_config::get();
	auto isOnScreen = [](Vector origin, Vector& screen) -> bool
	{
		if (!math::world_to_screen(origin, screen))
			return false;

		static int iScreenWidth, iScreenHeight;
		m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

		auto xOk = iScreenWidth > screen.x;
		auto yOk = iScreenHeight > screen.y;

		return xOk && yOk;
	};

	Vector screenPos;

	if (isOnScreen(e->GetAbsOrigin(), screenPos))
		return;

	Vector viewAngles;
	m_engine()->GetViewAngles(viewAngles);

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto screenCenter = Vector2D(width * 0.5f, height * 0.5f);
	auto angleYawRad = DEG2RAD(viewAngles.y - math::calculate_angle(g_ctx.globals.eye_pos, e->GetAbsOrigin()).y - 90.0f);

	auto radius = cfg->i["oofradius"];
	auto size = cfg->i["oofsize"];

	auto newPointX = screenCenter.x + ((((width - (size * 4)) * 0.5f) * (radius / 200.0f)/*(radius / 100.0f)*/) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
	auto newPointY = screenCenter.y + ((((height - (size * 4)) * 0.5f) * (radius / 200.0f)/*(radius / 100.0f)*/) * sin(angleYawRad));

	std::array <Vector2D, 3> points
	{
		Vector2D(newPointX - size, newPointY - size),
			Vector2D(newPointX + size, newPointY),
			Vector2D(newPointX - size, newPointY + size)
	};

	math::rotate_triangle(points, viewAngles.y - math::calculate_angle(g_ctx.globals.eye_pos, e->GetAbsOrigin()).y - 90.0f);
	Render::Draw->Triangle( IdaLovesMe::Vec2::Vec2(points.at( 0 ).x, points.at( 0 ).y) , IdaLovesMe::Vec2::Vec2( points.at( 1 ).x, points.at( 1 ).y ), IdaLovesMe::Vec2::Vec2( points.at( 2 ).x, points.at( 2 ).y ), D3DCOLOR_RGBA( color.r(), color.g(), color.b(), color.a() ) );
}

void misc::NightmodeFix()
{
	static auto in_game = false;

	if (m_engine()->IsInGame() && !in_game)
	{
		in_game = true;

		g_ctx.globals.change_materials = true;
		worldesp::get().changed = true;

		static auto skybox = m_cvar()->FindVar(crypt_str("sv_skyname"));
		worldesp::get().backup_skybox = skybox->GetString();
		return;
	}
	else if (!m_engine()->IsInGame() && in_game)
		in_game = false;

	static auto player_enable = c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]);

	if (player_enable != c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
	{
		player_enable = c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]);
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting = c_config::get()->m["brightadj"][1];

	if (setting != c_config::get()->m["brightadj"][1])
	{
		setting = c_config::get()->m["brightadj"][1];
		g_ctx.globals.change_materials = true;
		return;
	}

	Color wrldcol
	{
		c_config::get()->c["adjcol"][0],
		c_config::get()->c["adjcol"][1],
		c_config::get()->c["adjcol"][2],
		//( int ) ( c_config::get( )->i[ "transwalls" ] * 2.55 )
		c_config::get()->c["adjcol"][3]
	};

	static auto setting_world = wrldcol;

	if (setting_world != wrldcol)
	{
		setting_world = wrldcol;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting_props = Color(255, 255, 255, c_config::get()->i["transprops"]);

	if (setting_props != Color(255, 255, 255, c_config::get()->i["transprops"]))
	{
		setting_props = Color(255, 255, 255, c_config::get()->i["transprops"] * 2.55);
		g_ctx.globals.change_materials = true;
	}

	/*static auto setting_props = g_cfg.esp.props_color;

	if (setting_props != g_cfg.esp.props_color)
	{
		setting_props = g_cfg.esp.props_color;
		g_ctx.globals.change_materials = true;
	}*/
}

void misc::aimbot_hitboxes()
{
	if (!c_config::get()->auto_check(c_config::get()->i["esp_en"], c_config::get()->i["esp_en_type"]))
		return;

	if (!c_config::get()->b["onshot"])
		return;

	auto player = (player_t*)m_entitylist()->GetClientEntity(aim::get().last_target_index);

	if (!player)
		return;

	auto model = player->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto hitbox_set = studio_model->pHitboxSet(player->m_nHitboxSet());

	if (!hitbox_set)
		return;

	hit_chams::get().add_matrix(player, aim::get().last_target[aim::get().last_target_index].record.matrixes_data.main);
}

void misc::rank_reveal()
{
	if (!c_config::get()->b["misc_rev_comp"])
		return;

	using RankReveal_t = bool(__cdecl*)(int*);
	static auto Fn = (RankReveal_t)(util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 A1 ? ? ? ? 85 C0 75 37")));

	int array[3] =
	{
		0,
		0,
		0
	};

	Fn(array);
}

void misc::fast_stop(CUserCmd* m_pcmd)
{
	if (!c_config::get()->b["rage_quickstop"])
		return;

	if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	auto pressed_move_key = m_pcmd->m_buttons & IN_FORWARD || m_pcmd->m_buttons & IN_MOVELEFT || m_pcmd->m_buttons & IN_BACK || m_pcmd->m_buttons & IN_MOVERIGHT || m_pcmd->m_buttons & IN_JUMP;

	if (pressed_move_key)
		return;

	if (!(g_cfg.antiaim.type[antiaim::get().type].desync && !g_cfg.antiaim.lby_type && (!g_ctx.globals.weapon->is_grenade() || g_cfg.esp.on_click && ~(m_pcmd->m_buttons & IN_ATTACK) && !(m_pcmd->m_buttons & IN_ATTACK2))) || antiaim::get().condition(m_pcmd)) //-V648
	{
		auto& velocity = g_ctx.local()->m_vecVelocity();

		if (velocity.Length2D() > 20.0f)
		{
			Vector direction;
			Vector real_view;

			math::vector_angles(velocity, direction);
			m_engine()->GetViewAngles(real_view);

			direction.y = real_view.y - direction.y;

			Vector forward;
			math::angle_vectors(direction, forward);

			static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
			static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));

			auto negative_forward_speed = -cl_forwardspeed->GetFloat();
			auto negative_side_speed = -cl_sidespeed->GetFloat();

			auto negative_forward_direction = forward * negative_forward_speed;
			auto negative_side_direction = forward * negative_side_speed;

			m_pcmd->m_forwardmove = negative_forward_direction.x;
			m_pcmd->m_sidemove = negative_side_direction.y;


		}
	}
	else
	{
		auto& velocity = g_ctx.local()->m_vecVelocity();

		if (velocity.Length2D() > 20.0f)
		{
			Vector direction;
			Vector real_view;

			math::vector_angles(velocity, direction);
			m_engine()->GetViewAngles(real_view);

			direction.y = real_view.y - direction.y;

			Vector forward;
			math::angle_vectors(direction, forward);

			static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
			static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));

			auto negative_forward_speed = -cl_forwardspeed->GetFloat();
			auto negative_side_speed = -cl_sidespeed->GetFloat();

			auto negative_forward_direction = forward * negative_forward_speed;
			auto negative_side_direction = forward * negative_side_speed;

			m_pcmd->m_forwardmove = negative_forward_direction.x;
			m_pcmd->m_sidemove = negative_side_direction.y;
		}
		else
		{
			auto speed = 1.01f;

			if (m_pcmd->m_buttons & IN_DUCK || g_ctx.globals.fakeducking)
				speed *= 2.94117647f;

			static auto switch_move = false;

			if (switch_move)
				m_pcmd->m_sidemove += speed;
			else
				m_pcmd->m_sidemove -= speed;

			switch_move = !switch_move;
		}
	}
}

void misc::spectators_list()
{
	if (!c_config::get()->b["spectators"])
		return;

	if (!g_ctx.local()->is_alive())
		return;

	std::vector <std::string> spectators;

	for (int i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->is_alive())
			continue;

		if (e->IsDormant())
			continue;

		if (e->m_hObserverTarget().Get() != g_ctx.local())
			continue;

		player_info_t player_info;
		m_engine()->GetPlayerInfo(i, &player_info);

		spectators.push_back(player_info.szName);
	}

	for (auto i = 0; i < spectators.size(); i++)
	{
		int width, heigth;
		m_engine()->GetScreenSize(width, heigth);

		auto x = render::get().text_width(fonts[NAME], spectators.at(i).c_str()) + 6; //-V106
		auto y = i * 16;

		//render::get().text(fonts[NAME], width - x, c_config::get()->b["spectators"] ? y + 30 : y + 6, Color::White, HFONT_CENTERED_NONE, spectators.at(i).c_str()); //-V106

		Render::Draw->Text_test( spectators.at( i ).c_str( ), width - x , c_config::get( )->b[ "spectators" ] ? y + 30 : y + 6 , 0, CMenu::get()->GetFontDPI(), false, D3DCOLOR_RGBA(255, 255, 255, 210));
	}
}

enum key_bind_num
{
	_AUTOFIRE,
	_LEGITBOT,
	_DOUBLETAP,
	_SAFEPOINT,
	_MIN_DAMAGE,
	_ANTI_BACKSHOT = 12,
	_M_BACK,
	_M_LEFT,
	_M_RIGHT,
	_DESYNC_FLIP,
	_THIRDPERSON,
	_AUTO_PEEK,
	_EDGE_JUMP,
	_FAKEDUCK,
	_SLOWWALK,
	_BODY_AIM,
	_RAGEBOT,
	_TRIGGERBOT,
	_L_RESOLVER_OVERRIDE,
	_FAKE_PEEK,
};

bool can_shift_shot(int ticks)
{
	if (!g_ctx.local() || !g_ctx.local()->m_hActiveWeapon())
		return false;

	auto tickbase = g_ctx.local()->m_nTickBase();
	auto curtime = m_globals()->m_intervalpertick * (tickbase - ticks);

	if (curtime < g_ctx.local()->m_flNextAttack())
		return false;

	if (curtime < g_ctx.local()->m_hActiveWeapon()->m_flNextPrimaryAttack())
		return false;

	return true;
}

void shift_cmd(CUserCmd* cmd, int amount)
{
	int cmd_number = cmd->m_command_number;
	int cnt = cmd_number - 150 * ((cmd_number + 1) / 150) + 1;
	auto new_cmd = &m_input()->m_pCommands[cnt];


	auto net_chan = m_clientstate()->pNetChannel;
	if (!new_cmd || !net_chan)
		return;

	std::memcpy(new_cmd, cmd, sizeof(CUserCmd));

	new_cmd->m_command_number = cmd->m_command_number + 1;
	new_cmd->m_buttons &= ~0x801u;


	for (int i = 0; i < amount; ++i)
	{
		int cmd_num = new_cmd->m_command_number + i;

		auto cmd_ = m_input()->GetUserCmd(cmd_num);
		auto verified_cmd = m_input()->GetVerifiedUserCmd(cmd_num);

		std::memcpy(cmd_, new_cmd, sizeof(CUserCmd));

		if (cmd_->m_tickcount != INT_MAX && m_clientstate()->iDeltaTick > 0)
			m_prediction()->Update(m_clientstate()->iDeltaTick, true, m_clientstate()->nLastCommandAck, m_clientstate()->nLastOutgoingCommand + m_clientstate()->iChokedCommands);

		cmd_->m_command_number = cmd_num;
		cmd_->m_predicted = cmd_->m_tickcount != INT_MAX;

		std::memcpy(verified_cmd, cmd_, sizeof(CUserCmd));
		verified_cmd->m_crc = cmd_->GetChecksum();

		++m_clientstate()->iChokedCommands;
		++net_chan->m_nChokedPackets;
		++net_chan->m_nOutSequenceNr;
	}

	*(int*)((uintptr_t)m_prediction() + 0xC) = -1;
	*(int*)((uintptr_t)m_prediction() + 0x1C) = 0;
}

void misc::shift_on_peek(CUserCmd* m_pcmd)
{
	//static int shift_amount = 0;
	static int choke_timer = 0;
	//static bool inCycle = false;
	//g_ctx.globals.m_Peek.m_bIsPeeking = false;
	if (!m_clientstate()->pNetChannel)
		return;


	for (auto i = 1; i <= m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));
		if (!e->valid(true))
			continue;

		auto records = &player_records[i];
		if (records->empty())
			continue;

		auto record = &records->front();
		if (!record->valid())
			continue;

		// apply player animated data
		record->adjust_player();

		// look all ticks for get first hitable
		Vector predicted_eye_pos = g_ctx.globals.eye_pos + (engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick);
		const float predict_shift = 10;
		for (int next_shift = 1; next_shift < (predict_shift); ++next_shift)
		{
			predicted_eye_pos += (engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick);
			auto boxes = aim::get().get_hitboxes(record, true);
			for (auto box : boxes)
			{
				if (g_ctx.globals.m_Peek.m_bIsPeeking)
					break;
				auto points = aim::get().get_points(record, box);
				for (auto point : points)
				{
					auto fire_data = autowall::get().wall_penetration(predicted_eye_pos, point.point, e);
					if (fire_data.valid)
					{
						g_ctx.globals.m_Peek.m_bIsPeeking = true;

						break;
					}
				}
			}
		}
	}

	if (g_ctx.globals.m_Peek.m_bIsPeeking)
	{
		g_ctx.globals.tickbase_shift = 2;
		g_ctx.globals.shift_command = m_pcmd->m_command_number;
		g_ctx.globals.shift_amount = 2;
		g_ctx.globals.m_Peek.m_bIsPeeking = false;
	}
}


bool misc::double_tap(CUserCmd* m_pcmd)
{
	static float lastdoubletaptime = 0.0f;
	if (!c_config::get()->b["rage_dt"] || (c_config::get()->auto_check(c_config::get()->i["rage_dt_key"], c_config::get()->i["rage_dt_key_style"]) <= KEY_NONE || c_config::get()->auto_check(c_config::get()->i["rage_dt_key"], c_config::get()->i["rage_dt_key_style"]) >= KEY_MAX) || !c_config::get()->auto_check(c_config::get()->i["rage_dt_key"], c_config::get()->i["rage_dt_key_style"]))
	{
		g_ctx.globals.double_tap_aim = false;
		g_ctx.globals.double_tap_aim_check = false;
		g_ctx.globals.shift_ticks = g_ctx.globals.tocharge;
		return false;
	}

	auto weapon = g_ctx.local()->m_hActiveWeapon();



	if (!(m_pcmd->m_buttons & IN_ATTACK) && g_ctx.globals.tocharge < 14 && abs(g_ctx.globals.fixed_tickbase - lastdoubletaptime) >= TIME_TO_TICKS(0.5)) {
		this->double_tap_enabled = false;
		g_ctx.globals.should_recharge = true;
		g_ctx.globals.tochargeamount = 14;

	}
	else {
		this->double_tap_enabled = true;

		g_ctx.globals.should_recharge = false;
	}


	if (g_ctx.globals.tocharge > 14) {
		g_ctx.globals.shift_ticks = g_ctx.globals.tocharge - 13;
	}

	if (weapon && (m_pcmd->m_buttons & IN_ATTACK || (m_pcmd->m_buttons & IN_ATTACK2 && weapon->is_knife())) && g_ctx.globals.tocharge >= 14) {
		lastdoubletaptime = g_ctx.globals.fixed_tickbase;

		g_ctx.globals.tickbase_shift = 13;
		g_ctx.globals.shift_ticks = 13;
		g_ctx.globals.shot_command = g_ctx.get_command()->m_command_number;
	}

	if (g_ctx.globals.aimbot_working)
	{
		g_ctx.globals.double_tap_aim = true;
		g_ctx.globals.double_tap_aim_check = true;
	}
	if (g_ctx.globals.tickbase_shift <= 0)
		g_ctx.globals.tickbase_shift = g_ctx.globals.shift_ticks;

	return true;
}


void misc::hide_shots(CUserCmd* m_pcmd, bool should_work)
{
	hide_shots_enabled = true;

	if ((c_config::get()->b["rage_enabled"] && !c_config::get()->auto_check(c_config::get()->i["rage_key_enabled"], c_config::get()->i["rage_key_enabled_st"])))
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		if (should_work)
		{
			g_ctx.globals.ticks_allowed = 0;
			g_ctx.globals.next_tickbase_shift = 0;
		}

		return;
	}

	if (!should_work && double_tap_key)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		return;
	}

	if (!hide_shots_key)
	{
		hide_shots_enabled = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.next_tickbase_shift = 0;
		return;
	}

	double_tap_key = false;

	if (g_ctx.local()->m_bGunGameImmunity() || g_ctx.local()->m_fFlags() & FL_FROZEN)
	{
		hide_shots_enabled = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.next_tickbase_shift = 0;
		return;
	}

	if (g_ctx.globals.fakeducking)
	{
		hide_shots_enabled = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.next_tickbase_shift = 0;
		return;
	}

	if (antiaim::get().freeze_check)
		return;

	g_ctx.globals.next_tickbase_shift = m_gamerules()->m_bIsValveDS() ? 6 : 9;

	auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);
	auto weapon_shoot = m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || m_pcmd->m_buttons & IN_ATTACK2 && g_ctx.globals.weapon->is_knife() || revolver_shoot;

	if (g_ctx.send_packet && !g_ctx.globals.weapon->is_grenade() && weapon_shoot)
		g_ctx.globals.tickbase_shift = g_ctx.globals.next_tickbase_shift;
}