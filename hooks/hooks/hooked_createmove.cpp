// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\cheats\ragebot\antiaim.h"
#include "..\cheats\visuals\other_esp.h"
#include "..\cheats\misc\fakelag.h"
#include "..\cheats\misc\prediction_system.h"
#include "..\..\cheats\misc\simulation.h"
#include "..\cheats\ragebot\aim.h"
#include "..\cheats\legitbot\legitbot.h"
#include "..\cheats\misc\bunnyhop.h"
#include "..\cheats\misc\airstrafe.h"
#include "..\cheats\misc\spammers.h"
#include "..\cheats\fakewalk\slowwalk.h"
#include "..\cheats\misc\misc.h"
#include "..\cheats\misc\logs.h"
#include "..\cheats\visuals\GrenadePrediction.h"
#include "..\cheats\ragebot\knifebot.h"
#include "..\cheats\ragebot\zeusbot.h"
#include "..\cheats\lagcompensation\local_animations.h"
#include "..\cheats\lagcompensation\animation_system.h"

using CreateMove_t = void(__stdcall*)(int, float, bool);

void __stdcall hooks::hooked_createmove(int sequence_number, float input_sample_frametime, bool active, bool& send_packet)
{
	static auto original_fn = client_hook->get_func_address <CreateMove_t>(22);
	original_fn(sequence_number, input_sample_frametime, active);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	g_ctx.globals.in_createmove = true;
	auto m_pcmd = m_input()->GetUserCmd(sequence_number);
	auto verified = m_input()->GetVerifiedUserCmd(sequence_number);

	if (!m_pcmd || !m_pcmd->m_command_number)
		return;

	if (!g_ctx.available())
		return;

	misc::get().rank_reveal();
	spammers::get().clan_tag();

	if (c_config::get()->b["misc_hidden_cvars"])
		misc::get().EnableHiddenCVars();

	if (m_engine()->IsInGame() && m_engine()->IsConnected())
		misc::get().pingspike();

	if (!g_ctx.local()->is_alive()) //-V807
		return;

	g_ctx.globals.weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!g_ctx.globals.weapon)
		return;

	g_ctx.set_command(m_pcmd);

	if (menu_open && g_ctx.globals.focused_on_input)
	{
		m_pcmd->m_buttons = 0;
		m_pcmd->m_forwardmove = 0.0f;
		m_pcmd->m_sidemove = 0.0f;
		m_pcmd->m_upmove = 0.0f;
	}

	//if (g_ctx.globals.should_recharge)
	//{
	//	m_pcmd->m_tickcount = INT_MAX;
	//	m_pcmd->m_forwardmove = 0.0f;
	//	m_pcmd->m_sidemove = 0.0f;
	//	m_pcmd->m_upmove = 0.0f;
	//	m_pcmd->m_buttons &= ~IN_ATTACK;
	//	m_pcmd->m_buttons &= ~IN_ATTACK2;

	//	verified->m_cmd = *m_pcmd;
	//	verified->m_crc = m_pcmd->GetChecksum();
	//	return;
	//}

	if (g_ctx.globals.ticks_allowed < 13 && ((misc::get().double_tap_enabled && misc::get().double_tap_key) || (misc::get().hide_shots_enabled && misc::get().hide_shots_key)))
		g_ctx.globals.should_recharge = true;


	//if (g_ctx.globals.should_recharge)
	//{
	//	m_pcmd->m_tickcount = INT_MAX;
	//	m_pcmd->m_forwardmove = 0.0f;
	//	m_pcmd->m_sidemove = 0.0f;
	//	m_pcmd->m_upmove = 0.0f;
	//	m_pcmd->m_buttons &= ~IN_ATTACK;
	//	m_pcmd->m_buttons &= ~IN_ATTACK2;

	//	verified->m_cmd = *m_pcmd;
	//	verified->m_crc = m_pcmd->GetChecksum();
	//	return;
	//}



	if (misc::get().hide_shots_enabled)
		g_ctx.globals.fixed_tickbase = g_ctx.local()->m_nTickBase() - 6;
	else
		g_ctx.globals.fixed_tickbase = g_ctx.local()->m_nTickBase();

	if (menu_open)
	{
		m_pcmd->m_buttons &= ~IN_ATTACK;
		m_pcmd->m_buttons &= ~IN_ATTACK2;
	}

	if (m_pcmd->m_buttons & IN_ATTACK2 && c_config::get()->b["rage_enabled"] && g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		m_pcmd->m_buttons &= ~IN_ATTACK2;

	if (c_config::get()->b["rage_enabled"] && !g_ctx.globals.weapon->can_fire(true))
	{
		if (m_pcmd->m_buttons & IN_ATTACK && !g_ctx.globals.weapon->is_non_aim() && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			m_pcmd->m_buttons &= ~IN_ATTACK;
		else if ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && (g_ctx.globals.weapon->is_knife()
			|| g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_HEALTHSHOT)
		{
			if (m_pcmd->m_buttons & IN_ATTACK)
				m_pcmd->m_buttons &= ~IN_ATTACK;

			if (m_pcmd->m_buttons & IN_ATTACK2)
				m_pcmd->m_buttons &= ~IN_ATTACK2;
		}
	}

	if (m_pcmd->m_buttons & IN_FORWARD && m_pcmd->m_buttons & IN_BACK)
	{
		m_pcmd->m_buttons &= ~IN_FORWARD;
		m_pcmd->m_buttons &= ~IN_BACK;
	}

	if (m_pcmd->m_buttons & IN_MOVELEFT && m_pcmd->m_buttons & IN_MOVERIGHT)
	{
		m_pcmd->m_buttons &= ~IN_MOVELEFT;
		m_pcmd->m_buttons &= ~IN_MOVERIGHT;
	}

	g_ctx.send_packet = true;
	g_ctx.globals.tickbase_shift = 0;
	g_ctx.globals.double_tap_fire = false;
	g_ctx.globals.force_send_packet = false;
	g_ctx.globals.exploits = c_config::get()->auto_check(c_config::get()->i["rage_dt_key"], c_config::get()->i["rage_dt_key_style"]) || c_config::get()->auto_check(c_config::get()->i["hs_key"], c_config::get()->i["hs_key_style"]);
	g_ctx.globals.current_weapon = g_ctx.globals.weapon->get_weapon_group(g_cfg.ragebot.enable);
	g_ctx.globals.slowwalking = false;
	g_ctx.globals.original_forwardmove = m_pcmd->m_forwardmove;
	g_ctx.globals.original_sidemove = m_pcmd->m_sidemove;
	antiaim::get().breaking_lby = false;
	//g_ctx.globals.tickrate = 1.f / m_globals()->m_intervalpertick;

	auto wish_yaw = m_pcmd->m_viewangles;
	misc::get().fast_stop(m_pcmd);

	if (c_config::get()->b["bhop"])
		bunnyhop::get().create_move();

	misc::get().SlideWalk(m_pcmd);
	misc::get().NoDuck(m_pcmd);

	misc::get().AutoCrouch(m_pcmd);
	GrenadePrediction::get().Tick(m_pcmd->m_buttons);

	if (c_config::get()->i["airducksel"] == 1 && !(g_ctx.local()->m_fFlags() & FL_ONGROUND))
		m_pcmd->m_buttons |= IN_DUCK;

	engineprediction::get().prediction_data.reset();
	engineprediction::get().setup();

	engineprediction::get().predict(m_pcmd);
	local_animations::get().update_prediction_animations();

	g_ctx.globals.eye_pos = g_ctx.local()->get_shoot_position();

	if (c_config::get()->b["air_starfe"])
		airstrafe::get().create_move(m_pcmd);

	if (key_binds::get().get_key_bind_state(19) && engineprediction::get().backup_data.flags & FL_ONGROUND && !(g_ctx.local()->m_fFlags() & FL_ONGROUND))
		m_pcmd->m_buttons |= IN_JUMP;

	if (key_binds::get().get_key_bind_state(21))
		slowwalk::get().create_move(m_pcmd);

	if (c_config::get()->b["rage_enabled"] && !g_ctx.globals.weapon->is_non_aim() && engineprediction::get().backup_data.flags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND)
		slowwalk::get().create_move(m_pcmd, 0.95f + 0.003125f * (16 - m_clientstate()->iChokedCommands));

	if (!g_ctx.globals.should_recharge)
		fakelag::get().Createmove();

	g_ctx.globals.aimbot_working = false;
	g_ctx.globals.revolver_working = false;

	auto backup_velocity = g_ctx.local()->m_vecVelocity();
	auto backup_abs_velocity = g_ctx.local()->m_vecAbsVelocity();

	g_ctx.local()->m_vecVelocity() = engineprediction::get().backup_data.velocity;
	g_ctx.local()->m_vecAbsVelocity() = engineprediction::get().backup_data.velocity;

	g_ctx.globals.weapon->update_accuracy_penality();

	g_ctx.local()->m_vecVelocity() = backup_velocity;
	g_ctx.local()->m_vecAbsVelocity() = backup_abs_velocity;

	g_ctx.globals.inaccuracy = g_ctx.globals.weapon->get_inaccuracy();
	g_ctx.globals.spread = g_ctx.globals.weapon->get_spread();

	aim::get().run(m_pcmd);
	zeusbot::get().run(m_pcmd);
	knifebot::get().run(m_pcmd);

	antiaim::get().desync_angle = 0.0f;
	antiaim::get().create_move(m_pcmd);

	if (m_clientstate()->iChokedCommands >= (m_gamerules()->m_bIsValveDS() ? 6 : 14))
		g_ctx.send_packet = true;

	if (g_ctx.globals.should_send_packet)
	{
		g_ctx.globals.force_send_packet = true;
		g_ctx.send_packet = true;
	}

	if (g_ctx.globals.should_choke_packet)
	{
		g_ctx.globals.should_choke_packet = false;
		g_ctx.globals.should_send_packet = true;
		g_ctx.send_packet = false;
	}

	if (!g_ctx.globals.weapon->is_non_aim() && g_ctx.globals.weapon->can_fire(true))
	{
		auto rapid_fire_aim_check = false;

		if (m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.double_tap_aim_check)
		{
			rapid_fire_aim_check = true;
			g_ctx.globals.double_tap_aim_check = false;
		}

		auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

		if (m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
		{
			static auto weapon_recoil_scale = m_cvar()->FindVar(crypt_str("weapon_recoil_scale"));

			if (c_config::get()->b["rage_enabled"])
				m_pcmd->m_viewangles -= g_ctx.local()->m_aimPunchAngle() * weapon_recoil_scale->GetFloat();

			if (!g_ctx.globals.fakeducking)
			{
				g_ctx.globals.force_send_packet = true;
				g_ctx.globals.should_choke_packet = true;
				g_ctx.send_packet = true;
			}

			aim::get().last_shoot_position = g_ctx.globals.eye_pos;

			if (!rapid_fire_aim_check)
				g_ctx.globals.double_tap_aim = false;
		}
	}
	else if (!g_ctx.globals.fakeducking && g_ctx.globals.weapon->is_knife() && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2))
	{
		g_ctx.globals.force_send_packet = true;
		g_ctx.globals.should_choke_packet = true;
		g_ctx.send_packet = true;
	}

	if (g_ctx.globals.fakeducking)
		g_ctx.globals.force_send_packet = g_ctx.send_packet;

	for (auto& backup : aim::get().backup)
		backup.adjust_player();

	auto backup_ticks_allowed = g_ctx.globals.ticks_allowed;

	if (misc::get().double_tap(m_pcmd)) {
		misc::get().shift_on_peek(m_pcmd);
		misc::get().hide_shots(m_pcmd, false);
	}
	else
	{
		g_ctx.globals.ticks_allowed = backup_ticks_allowed;
		misc::get().hide_shots(m_pcmd, true);
	}

	SimulationContext::get( ).InitSimulationContext( g_ctx.local( ) );

	SimulationContext::get( ).RebuildGameMovement( m_pcmd );

	misc::get().automatic_peek(m_pcmd, wish_yaw.y);

	if (m_globals()->m_tickcount - g_ctx.globals.last_aimbot_shot > 16) //-V807
	{
		g_ctx.globals.double_tap_aim = false;
		g_ctx.globals.double_tap_aim_check = false;
	}

	engineprediction::get().finish();

	if (c_config::get()->b["anti_untrusted"])
		math::normalize_angles(m_pcmd->m_viewangles);
	else
		m_pcmd->m_viewangles.y = math::normalize_yaw(m_pcmd->m_viewangles.y);

	if (g_ctx.globals.should_recharge)
		g_ctx.send_packet = true;

	if (g_ctx.globals.loaded_script)
		for (auto current : c_lua::get().hooks.getHooks(crypt_str("on_createmove")))
			current.func();

	if (g_ctx.send_packet && !g_ctx.globals.should_send_packet && (!g_ctx.globals.should_choke_packet || (!misc::get().hide_shots_enabled && !g_ctx.globals.double_tap_fire)))
	{
		local_animations::get().local_data.fake_angles = m_pcmd->m_viewangles; //-V807
		local_animations::get().local_data.real_angles = local_animations::get().local_data.stored_real_angles;
	}

	if (!antiaim::get().breaking_lby)
		local_animations::get().local_data.stored_real_angles = m_pcmd->m_viewangles;

	if (g_ctx.globals.should_recharge)
		g_ctx.send_packet = true;

	if (g_ctx.globals.ticks_choke)
	{
		g_ctx.send_packet = g_ctx.globals.force_send_packet;
		--g_ctx.globals.ticks_choke;
	}

	util::movement_fix(wish_yaw, m_pcmd);

	auto& correct = g_ctx.globals.data.emplace_front();

	correct.command_number = m_pcmd->m_command_number;
	correct.choked_commands = m_clientstate()->iChokedCommands + 1;
	correct.tickcount = m_globals()->m_tickcount;

	if (g_ctx.send_packet)
		g_ctx.globals.choked_number.clear();
	else
		g_ctx.globals.choked_number.emplace_back(correct.command_number);

	while (g_ctx.globals.data.size() > g_ctx.globals.tickrate)
		g_ctx.globals.data.pop_back();

	auto& out = g_ctx.globals.commands.emplace_back();

	out.is_outgoing = g_ctx.send_packet;
	out.is_used = false;
	out.command_number = m_pcmd->m_command_number;
	out.previous_command_number = 0;

	while (g_ctx.globals.commands.size() > g_ctx.globals.tickrate)
		g_ctx.globals.commands.pop_front();

	if (!g_ctx.send_packet && !m_gamerules()->m_bIsValveDS())
	{
		auto net_channel = m_clientstate()->pNetChannel;

		if (net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4))
		{
			auto backup_choke = net_channel->m_nChokedPackets;
			net_channel->m_nChokedPackets = 0;

			net_channel->send_datagram();
			--net_channel->m_nOutSequenceNr;

			net_channel->m_nChokedPackets = backup_choke;
		}
	}

	if (g_ctx.send_packet && g_ctx.globals.should_send_packet)
		g_ctx.globals.should_send_packet = false;


	if (send_packet)
		g_ctx.local()->m_angReal = m_pcmd->m_viewangles;

	send_packet = g_ctx.send_packet;
	g_ctx.globals.in_createmove = false;

	verified->m_cmd = *m_pcmd;
	verified->m_crc = m_pcmd->GetChecksum();
}

__declspec(naked) void __stdcall hooks::hooked_createmove_proxy(int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		push ebx
		push esp
		push dword ptr[esp + 20]
		push dword ptr[esp + 0Ch + 8]
		push dword ptr[esp + 10h + 4]
		call hooked_createmove
		pop ebx
		retn 0Ch
	}
}