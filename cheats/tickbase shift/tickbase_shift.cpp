//includes
#include "tickbase_shift.h"
#include "..\ragebot\aim.h"
//#include <Features\logging.h>
#include "../Misc/Fakelag.h"
#include "..\autowall/autowall.h"

//break lc
void exploit::BreakLagCompensation(CUserCmd* m_pcmd)
{

}

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

bool exploit::defensive(CUserCmd* m_pcmd)
{
	if (!g_ctx.globals.weapon->can_fire(true))
		return false;
	if (antiaim::get().freeze_check)
		return false;

	auto max_tickbase_shift = g_ctx.globals.weapon->get_max_tickbase_shift();
	auto velocity = engineprediction::get.backup_data.velocity = g_ctx.local()->m_vecVelocity();
	auto choked = m_clientstate()->iChokedCommands;
	static auto switch_ticks = false;
	static auto shift_peek_ticks = 0;

	static auto last_double_tap = 0;


	if (c_config::get()->i["rage_dt_key"] && c_config::get()->i["rage_dt_key_style"] > KEY_NONE && c_config::get()->i["rage_dt_key_style"] < KEY_MAX && exploit::get().double_tap_key)
	{
		if (c_config::get()->b["rage_dt"] && g_ctx.globals.current_weapon != -1 && !g_ctx.globals.m_Peek.started_peeking_Defensive && velocity >= 5.0f)
		{
			auto predicted_eye_pos = g_ctx.globals.eye_pos + engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick * (float)24.0f;

			for (auto i = 1; i < m_globals()->m_maxclients; i++)
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

				scan_data predicted_data;
				aim::get().scan(record, predicted_data, predicted_eye_pos);

				if (predicted_data.valid())
				{
					scan_data data;
					aim::get().scan(record, data, g_ctx.globals.eye_pos);

					if (!data.valid())
					{
						shift_peek_ticks = 15;
						g_ctx.globals.m_Peek.started_peeking_Defensive = true;

						g_ctx.send_packet = true;
					}
				}
			}
		}
		if (g_ctx.globals.m_Peek.started_peeking_Defensive)
		{
			++shift_peek_ticks;
			if (shift_peek_ticks > 15)
			{
				(g_ctx.globals.m_Peek.started_peeking_Defensive = false);
			}
		}
		else
		{
			shift_peek_ticks = 0;
		}

		g_ctx.globals.tickbase_shift = shift_peek_ticks;
		return true;
	}
	else
	{
		g_ctx.globals.m_Peek.started_peeking_Defensive = false;
	}
}


//dt
void exploit::DoubleTap(CUserCmd* m_pcmd)
{
	double_tap_enabled = true;
	//vars
	auto shiftAmount = 15;
	float shiftTime = shiftAmount * m_globals()->m_intervalpertick;
	float recharge_time = TIME_TO_TICKS(0.7);
	auto weapon = g_ctx.local()->m_hActiveWeapon();

	g_ctx.globals.tickbase_shift = shiftAmount;

	//Check if we can doubletap
	if (!CanDoubleTap(false))
		return;

	//Fix for doubletap hitchance
	if (g_ctx.globals.dt_shots == 1) {
		g_ctx.globals.dt_shots = 0;
	}

	//Recharge
	if (!aim::get().should_stop && !(m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2 && g_ctx.globals.weapon->is_knife())
		&& !util::is_button_down(MOUSE_LEFT) && g_ctx.globals.tocharge < shiftAmount && (m_pcmd->m_command_number - lastdoubletaptime) > recharge_time)
	{
		lastdoubletaptime = 0;
		g_ctx.globals.startcharge = true;
		g_ctx.globals.tochargeamount = shiftAmount;
		g_ctx.globals.dt_shots = 0;
	}
	else g_ctx.globals.startcharge = false;

	//Do the magic.
	bool restricted_weapon = (g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_TASER || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER
		|| g_ctx.globals.weapon->is_knife() || g_ctx.globals.weapon->is_grenade());
	if ((m_pcmd->m_buttons & IN_ATTACK) && !restricted_weapon && g_ctx.globals.tocharge == shiftAmount && weapon->m_flNextPrimaryAttack() <= m_pcmd->m_command_number - shiftTime) {
		if (g_ctx.globals.aimbot_working)
		{
			g_ctx.globals.double_tap_aim = true;
			g_ctx.globals.double_tap_aim_check = true;
		}

		g_ctx.globals.m_shifted_command = m_pcmd->m_command_number;
		lastdoubletaptime = m_pcmd->m_command_number;
		g_ctx.globals.shift_ticks = shiftAmount;
	}
}

bool exploit::CanDoubleTap(bool check_charge) {
	//check if DT key is enabled.
	if (!c_config::get()->b["rage_dt"] || c_config::get()->i["rage_dt_key_style"] <= KEY_NONE || c_config::get()->i["rage_dt_key_style"] >= KEY_MAX) {
		double_tap_enabled = false;
		double_tap_key = false;
		lastdoubletaptime = 0;
		g_ctx.globals.tocharge = 0;
		g_ctx.globals.tochargeamount = 0;
		g_ctx.globals.shift_ticks = 0;
		return false;
	}

	//if DT is on, disable hide shots.
	if (double_tap_key && c_config::get()->i["rage_dt_key_style"] != cfg::g_cfg.antiaim.hide_shots_key.key)
		hide_shots_key = false;

	//disable DT if frozen, fakeducking, revolver etc.
	if (!double_tap_key || g_ctx.local()->m_bGunGameImmunity() || g_ctx.local()->m_fFlags() & FL_FROZEN || m_gamerules()->m_bIsValveDS() || g_ctx.globals.fakeducking) {
		double_tap_enabled = false;
		lastdoubletaptime = 0;
		g_ctx.globals.tocharge = 0;
		g_ctx.globals.tochargeamount = 0;
		g_ctx.globals.shift_ticks = 0;

		return false;
	}

	if (check_charge) {
		if (g_ctx.globals.tochargeamount > 0)
			return false;

		if (g_ctx.globals.startcharge)
			return false;
	}

	return true;
}

void exploit::HideShots(CUserCmd* m_pcmd)
{
	if (double_tap_key)
		return;

	hide_shots_enabled = true;

	if (!c_config::get()->b["rage_dt"])
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.tickbase_shift = 0;

		return;
	}

	if (!cfg::g_cfg.antiaim.hide_shots)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.tickbase_shift = 0;

		return;
	}

	if (cfg::g_cfg.antiaim.hide_shots_key.key <= KEY_NONE || cfg::g_cfg.antiaim.hide_shots_key.key >= KEY_MAX)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.tickbase_shift = 0;

		return;
	}

	if (double_tap_key)
	{

		hide_shots_enabled = false;
		hide_shots_key = false;
		return;
	}

	if (!hide_shots_key)
	{
		hide_shots_enabled = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.tickbase_shift = 0;
		return;
	}

	double_tap_key = false;

	if (g_ctx.local()->m_bGunGameImmunity() || g_ctx.local()->m_fFlags() & FL_FROZEN)
	{
		hide_shots_enabled = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.tickbase_shift = 0;
		return;
	}

	if (g_ctx.globals.fakeducking)
	{
		hide_shots_enabled = false;
		g_ctx.globals.ticks_allowed = 0;
		g_ctx.globals.tickbase_shift = 0;
		return;
	}

	if (antiaim::get().freeze_check)
		return;

	g_ctx.globals.tickbase_shift = m_gamerules()->m_bIsValveDS() ? 6 : 9;

	auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);
	auto weapon_shoot = m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || m_pcmd->m_buttons & IN_ATTACK2 && g_ctx.globals.weapon->is_knife() || revolver_shoot;

	if (g_ctx.send_packet && !g_ctx.globals.weapon->is_grenade() && weapon_shoot)
		g_ctx.globals.tickbase_shift = g_ctx.globals.tickbase_shift;
}

