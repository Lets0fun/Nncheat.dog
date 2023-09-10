#include "aim.h"
#include "..\misc\misc.h"
#include "..\misc\logs.h"
#include "..\autowall\autowall.h"
#include "..\misc\prediction_system.h"
#include "..\fakewalk\slowwalk.h"
#include "..\lagcompensation\local_animations.h"

void aim::run(CUserCmd* cmd)
{
	backup.clear();
	targets.clear();
	scanned_targets.clear();
	final_target.reset();
	should_stop = false;

	if ((!c_config::get()->b["rage_enabled"]))
		return;

	if (!c_config::get()->auto_check(c_config::get()->i["rage_key_enabled"], c_config::get()->i["rage_key_enabled_st"]))
		return;

	automatic_revolver(cmd);
	prepare_targets();

	if (g_ctx.globals.weapon->is_non_aim())
		return;

	if (g_ctx.globals.current_weapon == -1)
		return;

	scan_targets();

	if (!should_stop)
	{
		auto max_speed = 260.0f;
		auto weapon_info = g_ctx.globals.weapon->get_csweapon_info();

		if (weapon_info)
			max_speed = g_ctx.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

		auto ticks_to_stop = math::clamp(engineprediction::get().backup_data.velocity.Length2D() / max_speed * 3.0f, 0.0f, 4.0f);
		auto predicted_eye_pos = g_ctx.globals.eye_pos + engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick * ticks_to_stop;

		for (auto& target : targets)
		{
			if (!target.last_record->valid())
				continue;

			scan_data last_data;

			target.last_record->adjust_player();
			scan(target.last_record, last_data, predicted_eye_pos, true);

			if (!last_data.valid())
				continue;

			should_stop = true;
			break;
		}
	}

	/* add check on current minimum damage and set should_stop to true this check help you. */

	if (!automatic_stop(cmd))
		return;

	if (scanned_targets.empty())
		return;

	find_best_target();

	if (!final_target.data.valid())
		return;

	fire(cmd);

	if (g_ctx.globals.loaded_script)
		for (auto current : c_lua::get().hooks.getHooks("player_hurt"))
			current.func();
}

void aim::automatic_revolver(CUserCmd* cmd)
{
	if (!m_engine()->IsActiveApp())
		return;

	if (g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
		return;

	if (cmd->m_buttons & IN_ATTACK)
		return;

	cmd->m_buttons &= ~IN_ATTACK2;

	static auto r8cock_time = 0.0f;
	auto server_time = TICKS_TO_TIME(g_ctx.globals.backup_tickbase);

	if (g_ctx.globals.weapon->can_fire(false))
	{
		if (r8cock_time <= server_time)
		{
			if (g_ctx.globals.weapon->m_flNextSecondaryAttack() <= server_time)
				r8cock_time = server_time + 0.2f/*0.234375f*/;
			else
				cmd->m_buttons |= IN_ATTACK2;
		}
		else
			cmd->m_buttons |= IN_ATTACK;
	}
	else
	{
		r8cock_time = server_time + 0.2f/*0.234375f*/;
		cmd->m_buttons &= ~IN_ATTACK;
	}

	g_ctx.globals.revolver_working = true;
}

void aim::prepare_targets()
{
	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		if (g_cfg.player_list.white_list[i])
			continue;

		auto e = (player_t*)m_entitylist()->GetClientEntity(i);

		if (!e->valid(true, false))
			continue;

		if (e->m_flSimulationTime() < e->m_flOldSimulationTime())
			continue;

		auto records = &player_records[i];

		if (records->empty())
			continue;

		targets.emplace_back(target(e, get_record(records, false), get_record(records, true)));
	}

	for (auto& target : targets)
		backup.emplace_back(adjust_data(target.e));


}

void aim::update_peek_state()
{
	g_ctx.globals.m_Peek.m_bIsPeeking = false;
	if ((!c_config::get()->b["rage_enabled"] && !c_config::get()->auto_check(c_config::get()->i["rage_key_enabled"], c_config::get()->i["rage_key_enabled_st"])) && (!c_config::get()->b["fakelag"] || !g_cfg.antiaim.triggers_fakelag_amount > 2))
		return;

	if (g_ctx.local()->m_vecVelocity().Length2D() > 5.0f)
		return;

	// predpos
	Vector predicted_eye_pos = g_ctx.globals.eye_pos + (engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick);

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
		for (int next_chock = 1; next_chock <= m_clientstate()->iChokedCommands; ++next_chock)
		{
			predicted_eye_pos *= next_chock;

			auto fire_data = autowall::get().wall_penetration(predicted_eye_pos, e->hitbox_position_matrix(HITBOX_HEAD, record->matrixes_data.first), e);
			if (!fire_data.valid || fire_data.damage < 1)
				continue;

			g_ctx.globals.m_Peek.m_bIsPeeking = true;
			m_debugoverlay()->AddBoxOverlay(predicted_eye_pos, Vector(-0.7f, -0.7f, -0.7f), Vector(0.7f, 0.7f, 0.7f), Vector(0.f, 0.f, 0.f), 0, 255, 0, 100, m_globals()->m_intervalpertick * 2);
		}
	}
}

static bool compare_records(const optimized_adjust_data& first, const optimized_adjust_data& second)
{
	auto first_pitch = math::normalize_pitch(first.angles.x);
	auto second_pitch = math::normalize_pitch(second.angles.x);

	if (fabs(first_pitch - second_pitch) > 15.0f)
		return fabs(first_pitch) < fabs(second_pitch);
	else if (first.duck_amount != second.duck_amount) //-V550
		return first.duck_amount < second.duck_amount;
	else if (first.origin != second.origin)
		return first.origin.DistTo(g_ctx.local()->GetAbsOrigin()) < second.origin.DistTo(g_ctx.local()->GetAbsOrigin());

	return first.simulation_time > second.simulation_time;
}

adjust_data* aim::get_record(std::deque <adjust_data>* records, bool history)
{
	if (history)
	{
		std::deque <optimized_adjust_data> optimized_records;

		for (auto i = 0; i < records->size(); ++i)
		{
			auto record = &records->at(i);
			optimized_adjust_data optimized_record;

			optimized_record.i = i;
			optimized_record.player = record->player;
			optimized_record.simulation_time = record->simulation_time;
			optimized_record.duck_amount = record->duck_amount;
			optimized_record.angles = record->angles;
			optimized_record.origin = record->origin;

			optimized_records.emplace_back(optimized_record);
		}

		if (optimized_records.size() < 2)
			return nullptr;

		std::sort(optimized_records.begin(), optimized_records.end(), compare_records);

		for (auto& optimized_record : optimized_records)
		{
			auto record = &records->at(optimized_record.i);

			if (!record->valid())
				continue;

			return record;
		}
	}
	else
	{
		for (auto i = 0; i < records->size(); ++i)
		{
			auto record = &records->at(i);

			if (!record->valid())
				continue;

			return record;
		}
	}

	return nullptr;
}

int aim::get_minimum_damage(bool visible, int health)
{
	auto minimum_damage = 1;
	minimum_damage = math::clamp(c_config::get()->i["min_hit_dam"], 1, health);


	return minimum_damage;
}

void aim::scan_targets()
{
	if (targets.empty())
		return;

	for (auto& target : targets)
	{
		if (target.history_record->valid())
		{
			scan_data first_tick;
			target.last_record->adjust_player();
			scan(target.last_record, first_tick);

			scan_data last_tick;
			target.history_record->adjust_player();
			scan(target.history_record, last_tick);

			if (first_tick.damage > last_tick.damage)
				scanned_targets.emplace_back(scanned_target(target.last_record, first_tick));
			else
				scanned_targets.emplace_back(scanned_target(target.history_record, last_tick));
		}
		else
		{
			if (!target.last_record->valid())
				continue;

			scan_data last_data;

			target.last_record->adjust_player();
			scan(target.last_record, last_data);

			if (!last_data.valid())
				continue;

			scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
		}
	}
}

bool aim::automatic_stop(CUserCmd* cmd)
{
	if (!should_stop)
		return true;

	if (!c_config::get()->b["rage_quickstop"])
		return true;

	if (!c_config::get()->b["rage_quickstop"])
		return true;

	if (g_ctx.globals.slowwalking)
		return true;

	if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND)) //-V807
		return true;

	if (g_ctx.globals.weapon->is_empty())
		return true;

	if (!c_config::get()->m["rage_quickstop_options"][4] && !g_ctx.globals.weapon->can_fire(false))
		return true;

	auto animlayer = g_ctx.local()->get_animlayers()[1];

	if (animlayer.m_nSequence)
	{
		auto activity = g_ctx.local()->sequence_activity(animlayer.m_nSequence);

		if (activity == ACT_CSGO_RELOAD && animlayer.m_flWeight > 0.0f)
			return true;
	}

	auto weapon_info = g_ctx.globals.weapon->get_csweapon_info();

	if (!weapon_info)
		return true;

	auto max_speed = 0.33f * (g_ctx.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed);

	if (engineprediction::get().backup_data.velocity.Length2D() < max_speed)
		slowwalk::get().create_move(cmd);
	else
	{
		Vector direction;
		Vector real_view;

		math::vector_angles(engineprediction::get().backup_data.velocity, direction);
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

		cmd->m_forwardmove = negative_forward_direction.x;
		cmd->m_sidemove = negative_side_direction.y;

		if (c_config::get()->m["rage_quickstop_options"][0])
			return false;
	}

	return true;
}

static bool compare_points(const scan_point& first, const scan_point& second)
{
	return !first.center && first.hitbox == second.hitbox;
}

void aim::scan(adjust_data* record, scan_data& data, const Vector& shoot_position, bool optimized)
{
	auto weapon = optimized ? g_ctx.local()->m_hActiveWeapon().Get() : g_ctx.globals.weapon;

	if (!weapon)
		return;

	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return;

	auto hitboxes = get_hitboxes(record, optimized);

	if (hitboxes.empty())
		return;

	//auto force_safe_points = key_binds::get().get_key_bind_state(3) || g_cfg.player_list.force_safe_points[record->i] || (c_config::get()->b["rage_safepoint"] && c_config::get()->i["rage_sp_enabled"]);// - V648
	//auto force_safe_points = record->player->m_iHealth() <= weapon_info->iDamage || key_binds::get().get_key_bind_state(3) || g_cfg.player_list.force_safe_points[record->i]; //-V648
	auto force_safe_points = key_binds::get().get_key_bind_state(3) || g_cfg.player_list.force_safe_points[record->i] || (c_config::get()->b["rage_safepoint"] && c_config::get()->i["rage_sp_enabled"]);
	auto best_damage = 0;

	auto minimum_damage = get_minimum_damage(false, record->player->m_iHealth());
	auto minimum_visible_damage = get_minimum_damage(true, record->player->m_iHealth());

	auto get_hitgroup = [](const int& hitbox)
	{
		if (hitbox == HITBOX_HEAD)
			return 0;
		else if (hitbox == HITBOX_PELVIS)
			return 1;
		else if (hitbox == HITBOX_STOMACH)
			return 2;
		else if (hitbox >= HITBOX_LOWER_CHEST && hitbox <= HITBOX_UPPER_CHEST)
			return 3;
		else if (hitbox >= HITBOX_RIGHT_THIGH && hitbox <= HITBOX_LEFT_FOOT)
			return 4;
		else if (hitbox >= HITBOX_RIGHT_HAND && hitbox <= HITBOX_LEFT_FOREARM)
			return 5;

		return -1;
	};


	std::vector <scan_point> points; //-V826

	for (auto& hitbox : hitboxes)
	{
		auto current_points = get_points(record, hitbox);

		for (auto& point : current_points)
		{
			if (!record->bot)
			{
				auto safe = 0.0f;

				if (record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.first[0].GetOrigin() || record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin() || record->matrixes_data.first[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin())
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.first, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.second, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;

				point.safe = safe;

				auto semi_safe = 0.0f;

				if (record->side == RESOLVER_DEFAULT || record->side == RESOLVER_ORIGINAL)
				{
					if (record->matrixes_data.low_first[0].GetOrigin() == record->matrixes_data.low_second[0].GetOrigin())
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.low_first, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.low_second, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;
				}
				else if (record->side == RESOLVER_FIRST)
				{
					if (record->matrixes_data.low_first[0].GetOrigin() == record->matrixes_data.zero[0].GetOrigin())
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.low_first, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;

					if (semi_safe == 0.0f)
					{
						if (!hitbox_intersection(record->player, record->matrixes_data.low_first, hitbox, shoot_position, point.point, &semi_safe))
							semi_safe = 0.0f;
					}
				}
				else if (record->side == RESOLVER_SECOND)
				{
					if (record->matrixes_data.low_second[0].GetOrigin() == record->matrixes_data.zero[0].GetOrigin())
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.low_second, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;

					if (semi_safe == 0.0f)
					{
						if (!hitbox_intersection(record->player, record->matrixes_data.low_second, hitbox, shoot_position, point.point, &semi_safe))
							semi_safe = 0.0f;
					}
				}
				else if (record->side == RESOLVER_LOW_FIRST)
				{
					if (record->matrixes_data.first[0].GetOrigin() == record->matrixes_data.zero[0].GetOrigin())
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.first, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;

					if (semi_safe == 0.0f)
					{
						if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &semi_safe))
							semi_safe = 0.0f;
					}
				}
				else if (record->side == RESOLVER_LOW_SECOND)
				{
					if (record->matrixes_data.second[0].GetOrigin() == record->matrixes_data.zero[0].GetOrigin())
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.second, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;
					else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &semi_safe))
						semi_safe = 0.0f;

					if (semi_safe == 0.0f)
					{
						if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &semi_safe))
							semi_safe = 0.0f;
					}
				}

				point.semi_safe = semi_safe;
			}
			else
				point.safe = 1.0f;

			if (!force_safe_points || point.safe)
				points.emplace_back(point);
		}
	}


	for (auto& point : points)
	{
		if (points.empty())
			return;

		if (point.hitbox == HITBOX_HEAD)
			continue;

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			if (point.point == it->point)
				continue;

			auto first_angle = math::calculate_angle(shoot_position, point.point);
			auto second_angle = math::calculate_angle(shoot_position, it->point);

			auto distance = shoot_position.DistTo(point.point);
			auto fov = math::fast_sin(DEG2RAD(math::get_fov(first_angle, second_angle))) * distance;

			if (fov < 5.0f)
			{
				points.erase(it);
				break;
			}
		}
	}


	if (points.empty())
		return;


	std::sort(points.begin(), points.end(), compare_points);

	auto body_hitboxes = true;

	for (auto& point : points)
	{
		if (body_hitboxes && (point.hitbox < HITBOX_CHEST || point.hitbox > HITBOX_CHEST))
		{
			body_hitboxes = false;

			if (best_damage >= record->player->m_iHealth())
				break;

			if (c_config::get()->auto_check(c_config::get()->i["rage_baim_enabled"], c_config::get()->i["rage_baim_enabled_style"]) && best_damage >= 1)
				break;
		}

		if (c_config::get()->b["rage_dt"] && c_config::get()->i["rage_dt_key"] > KEY_NONE && c_config::get()->i["rage_dt_key"] < KEY_MAX && c_config::get()->i["rage_dt_key"])
		{
			if (!g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && c_config::get()->b["rage_dt"] && !g_ctx.globals.weapon->is_grenade() && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_SSG08 && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_AWP)
			{
				if (best_damage * 1.939f >= record->player->m_iHealth())
					break;
			}
		}

		if ((c_config::get()->b["rage_safepoint"] || force_safe_points) && data.point.safe && data.point.safe > point.safe) //if ((c_config::get()->b["rage_safepoint"] || force_safe_points) && data.point.safe && data.point.safe < point.safe)
			continue;

		auto fire_data = autowall::get().wall_penetration(shoot_position, point.point, record->player);

		if (!fire_data.valid)
			continue;

		if (fire_data.damage < 1)
			continue;

		if (!fire_data.visible && !c_config::get()->b["auto_pen"])
			continue;

		if (get_hitgroup(fire_data.hitbox) != get_hitgroup(point.hitbox))
			continue;

		auto current_minimum_damage = minimum_damage;

		if (fire_data.damage >= current_minimum_damage && fire_data.damage >= best_damage)
		{
			if (!should_stop)
			{
				should_stop = true;

				if (g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].autostop_modifiers[AUTOSTOP_LETHAL] && fire_data.damage < record->player->m_iHealth())
					should_stop = false;
				else if (g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].autostop_modifiers[AUTOSTOP_VISIBLE] && !fire_data.visible)
					should_stop = false;
				else if (g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].autostop_modifiers[AUTOSTOP_CENTER] && !point.center)
					should_stop = false;
			}

			if (force_safe_points && !point.safe)
				continue;

			best_damage = fire_data.damage;

			data.point = point;
			data.visible = fire_data.visible;
			data.damage = fire_data.damage;
			data.hitbox = fire_data.hitbox;

			if (point.hitbox == HITBOX_HEAD && point.safe)
				break;

			if (c_config::get()->b["rage_safepoint"] && point.safe)
				break;

			if (point.hitbox == HITBOX_HEAD && point.semi_safe)
				break;

			if (c_config::get()->b["rage_safepoint"] && point.semi_safe)
				break;
		}
	}
}

std::vector <int> aim::get_hitboxes(adjust_data* record, bool optimized)
{
	std::vector <int> hitboxes; //-V827

	if (c_config::get()->auto_check(c_config::get()->i["rage_baim_enabled"], c_config::get()->i["rage_baim_enabled_style"]))
	{
		//hitboxes.emplace_back(HITBOX_UPPER_CHEST);
		hitboxes.emplace_back(HITBOX_CHEST);
		hitboxes.emplace_back(HITBOX_LOWER_CHEST);
		hitboxes.emplace_back(HITBOX_STOMACH);
		hitboxes.emplace_back(HITBOX_PELVIS);
		return hitboxes;
	}

	if (c_config::get()->m["rage_hitbox"].at(0))
		hitboxes.emplace_back(HITBOX_HEAD);

	if (c_config::get()->m["rage_hitbox"].at(1)) {
		hitboxes.emplace_back(HITBOX_UPPER_CHEST);
		hitboxes.emplace_back(HITBOX_CHEST);
	}

	if (c_config::get()->m["rage_hitbox"].at(2)) {
		hitboxes.emplace_back(HITBOX_STOMACH);
		hitboxes.emplace_back(HITBOX_PELVIS);
	}

	if (c_config::get()->m["rage_hitbox"].at(3))
	{
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
		hitboxes.emplace_back(HITBOX_RIGHT_FOREARM);
		hitboxes.emplace_back(HITBOX_RIGHT_HAND);

		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);
		hitboxes.emplace_back(HITBOX_LEFT_FOREARM);
		hitboxes.emplace_back(HITBOX_LEFT_HAND);
	}

	if (c_config::get()->m["rage_hitbox"].at(4))
	{
		hitboxes.emplace_back(HITBOX_RIGHT_THIGH);
		hitboxes.emplace_back(HITBOX_RIGHT_CALF);

		hitboxes.emplace_back(HITBOX_LEFT_THIGH);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
	}

	if (c_config::get()->m["rage_hitbox"].at(5))
	{
		hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_FOOT);
	}

	return hitboxes;
}

std::vector <scan_point> aim::get_points(adjust_data* record, int hitbox, bool from_aim)
{
	std::vector <scan_point> points; //-V827
	auto model = record->player->GetModel();

	if (!model)
		return points;

	auto hdr = m_modelinfo()->GetStudioModel(model);

	if (!hdr)
		return points;

	auto set = hdr->pHitboxSet(record->player->m_nHitboxSet());

	if (!set)
		return points;

	auto bbox = set->pHitbox(hitbox);

	if (!bbox)
		return points;

	auto center = (bbox->bbmin + bbox->bbmax) * 0.5f;

	if (bbox->radius <= 0.0f)
	{
		auto rotation_matrix = math::angle_matrix(bbox->rotation);

		matrix3x4_t matrix;
		math::concat_transforms(record->matrixes_data.main[bbox->bone], rotation_matrix, matrix);

		auto origin = matrix.GetOrigin();

		if (hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT)
		{
			auto side = (bbox->bbmin.z - center.z) * 0.875f;

			if (hitbox == HITBOX_LEFT_FOOT)
				side = -side;

			points.emplace_back(scan_point(Vector(center.x, center.y, center.z + side), hitbox, true));

			auto min = (bbox->bbmin.x - center.x) * 0.875f;
			auto max = (bbox->bbmax.x - center.x) * 0.875f;

			points.emplace_back(scan_point(Vector(center.x + min, center.y, center.z), hitbox, false));
			points.emplace_back(scan_point(Vector(center.x + max, center.y, center.z), hitbox, false));
		}
	}
	else
	{
		auto scale = 0.0f;

		if (c_config::get()->m["rage_mphitbox"][0] || c_config::get()->m["rage_mphitbox"][1] || c_config::get()->m["rage_mphitbox"][3] || c_config::get()->m["rage_mphitbox"][4])
		{
			if (hitbox == HITBOX_HEAD)
				scale = (int)c_config::get()->i["mp_scale"];
			else
				scale = (int)c_config::get()->i["mp_scale"];
		}

		if (c_config::get()->m["rage_mphitbox"][0] || c_config::get()->m["rage_mphitbox"][1] || c_config::get()->m["rage_mphitbox"][3] || c_config::get()->m["rage_mphitbox"][4])
		{
			float  v6;
			float pDist = 0.f;
			float flHitboxRadius;

			float g_flSpread = g_ctx.globals.spread;
			float g_flInaccurarcy = g_ctx.globals.inaccuracy;

			auto transformed_center = center;
			math::vector_transform(transformed_center, record->matrixes_data.main[bbox->bone], transformed_center);

			v6 = g_flSpread + g_flInaccurarcy;
			pDist = transformed_center.DistTo(g_ctx.globals.eye_pos) / std::sin(DEG2RAD(90.f - RAD2DEG(v6)));

			flHitboxRadius = max(bbox->radius - pDist * v6, 0.0f);
			scale = bbox->radius * math::clamp(flHitboxRadius - pDist * v6, 0.0f, 1.0f);
		}

		if (scale <= 0.0f) //-V648
		{
			math::vector_transform(center, record->matrixes_data.main[bbox->bone], center);
			points.emplace_back(scan_point(center, hitbox, true));

			return points;
		}

		auto final_radius = bbox->radius * scale;

		if (c_config::get()->m["rage_mphitbox"][0])
		{
			auto pitch_down = math::normalize_pitch(record->angles.x) > 85.0f;
			auto backward = fabs(math::normalize_yaw(record->angles.y - math::calculate_angle(record->player->get_shoot_position(), g_ctx.local()->GetAbsOrigin()).y)) > 120.0f;

			points.emplace_back(scan_point(center, hitbox, !pitch_down || !backward));

			points.emplace_back(scan_point(Vector(bbox->bbmax.x + 0.70710678f * final_radius, bbox->bbmax.y - 0.70710678f * final_radius, bbox->bbmax.z), hitbox, false));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y - final_radius, bbox->bbmax.z), hitbox, false));

			if (pitch_down && backward)
				points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
		}
		else if (c_config::get()->m["rage_mphitbox"][1])
		{
			points.emplace_back(scan_point(center, hitbox, true));

			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

			points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - final_radius, center.z), hitbox, true));
		}
		else if (c_config::get()->m["rage_mphitbox"][4])
		{
			points.emplace_back(scan_point(center, hitbox, true));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
		}
		else if (c_config::get()->m["rage_mphitbox"][4])
			points.emplace_back(scan_point(center, hitbox, true));
		else if (c_config::get()->m["rage_mphitbox"][3])
		{
			points.emplace_back(scan_point(center, hitbox, true));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x + final_radius, center.y, center.z), hitbox, false));
		}
	}

	for (auto& point : points)
		math::vector_transform(point.point, record->matrixes_data.main[bbox->bone], point.point);

	return points;
}

static bool compare_targets(const scanned_target& first, const scanned_target& second)
{
	return first.data.damage > second.data.damage;
}

void aim::find_best_target()
{
	std::sort(scanned_targets.begin(), scanned_targets.end(), compare_targets);

	for (auto& target : scanned_targets)
	{
		if (target.fov > (float)c_config::get()->i["rage_fov"])
			continue;

		final_target = target;
		final_target.record->adjust_player();
		break;
	}
}

void aim::fire(CUserCmd* cmd)
{
	if (!g_ctx.globals.weapon->can_fire(true))
		return;

	auto aim_angle = math::calculate_angle(g_ctx.globals.eye_pos, final_target.data.point.point).Clamp();

	if (!c_config::get()->b["rage_silent_enabled"] && (c_config::get()->b["rage_enabled"] || c_config::get()->i["rage_key_enabled"]))
		m_engine()->SetViewAngles(aim_angle);

	if (!c_config::get()->b["auto_fire"] && !(cmd->m_buttons & IN_ATTACK))
		return;

	auto hitchance_amount = 0;

	hitchance_amount = c_config::get()->i["min_hit_chan"];

	if (!hitchance(aim_angle, final_target.record->player, hitchance_amount, cmd))
	{
		if (c_config::get()->b["auto_scope"] && g_ctx.globals.weapon->is_sniper() && !g_ctx.globals.weapon->m_zoomLevel())
			cmd->m_buttons |= IN_ATTACK2;
		return;
	}

	auto backtrack_ticks = 0;
	auto net_channel_info = m_engine()->GetNetChannelInfo();

	if (net_channel_info)
	{
		auto original_tickbase = g_ctx.globals.backup_tickbase;
		auto max_tickbase_shift = m_gamerules()->m_bIsValveDS() ? 6 : 16;

		if (c_config::get()->b["rage_dt"] && c_config::get()->auto_check(c_config::get()->i["rage_dt_key"], c_config::get()->i["rage_dt_key_style"]))
		{
			if (!g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && c_config::get()->b["rage_dt"] && !g_ctx.globals.weapon->is_grenade() && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			{
				original_tickbase += min(g_ctx.globals.weapon->get_max_tickbase_shift(), max_tickbase_shift);
			}
		}

		if (c_config::get()->b["hideshots"] && c_config::get()->auto_check(c_config::get()->i["hs_key"], c_config::get()->i["hs_key_style"]))
		{
			if (!g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().hide_shots_enabled)
			{
				original_tickbase += min(9, max_tickbase_shift);
			}
		}

		static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

		auto correct = math::clamp(net_channel_info->GetLatency(FLOW_OUTGOING) + net_channel_info->GetLatency(FLOW_INCOMING) + util::get_interpolation(), 0.0f, sv_maxunlag->GetFloat());
		auto delta_time = correct - (TICKS_TO_TIME(original_tickbase) - final_target.record->simulation_time);

		backtrack_ticks = TIME_TO_TICKS(fabs(delta_time));
	}





	static auto get_hitbox_name = [](int hitbox, bool shot_info = false) -> std::string
	{
		switch (hitbox)
		{
		case HITBOX_HEAD:
			return shot_info ? crypt_str("Head") : crypt_str("head");
		case HITBOX_LOWER_CHEST:
			return shot_info ? crypt_str("Lower chest") : crypt_str("lower chest");
		case HITBOX_CHEST:
			return shot_info ? crypt_str("Chest") : crypt_str("chest");
		case HITBOX_UPPER_CHEST:
			return shot_info ? crypt_str("Upper chest") : crypt_str("upper chest");
		case HITBOX_STOMACH:
			return shot_info ? crypt_str("Stomach") : crypt_str("stomach");
		case HITBOX_PELVIS:
			return shot_info ? crypt_str("Pelvis") : crypt_str("pelvis");
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_RIGHT_HAND:
			return shot_info ? crypt_str("Left arm") : crypt_str("left arm");
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
		case HITBOX_LEFT_HAND:
			return shot_info ? crypt_str("Right arm") : crypt_str("right arm");
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
			return shot_info ? crypt_str("Left leg") : crypt_str("left leg");
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
			return shot_info ? crypt_str("Right leg") : crypt_str("right leg");
		case HITBOX_RIGHT_FOOT:
			return shot_info ? crypt_str("Left foot") : crypt_str("left foot");
		case HITBOX_LEFT_FOOT:
			return shot_info ? crypt_str("Right foot") : crypt_str("right foot");
		}
	};

	static auto get_resolver_type = [](resolver_type type) -> std::string
	{
		switch (type)
		{
		case ORIGINAL:
			return crypt_str("original ");
		case BRUTEFORCE:
			return crypt_str("bruteforce ");
		case LBY:
			return crypt_str("lby ");
		case TRACE:
			return crypt_str("trace ");
		case JITTER:
			return crypt_str("jitter ");
		case DIRECTIONAL:
			return crypt_str("directional ");
		case ANIMATION:
			return crypt_str("animation ");
		}
	};

	player_info_t player_info;
	m_engine()->GetPlayerInfo(final_target.record->i, &player_info);

	cmd->m_viewangles = aim_angle;
	cmd->m_buttons |= IN_ATTACK;
	cmd->m_tickcount = TIME_TO_TICKS(final_target.record->simulation_time + util::get_interpolation());

	last_target_index = final_target.record->i;
	last_shoot_position = g_ctx.globals.eye_pos;
	last_target[last_target_index] = Last_target
	{
		*final_target.record, final_target.data, final_target.distance
	};

	auto shot = &g_ctx.shots.emplace_back();

	shot->last_target = last_target_index;
	shot->fire_tick = m_globals()->m_tickcount;
	shot->shot_info.target_name = player_info.szName;
	shot->shot_info.client_hitbox = get_hitbox_name(final_target.data.hitbox, true);
	shot->shot_info.client_damage = final_target.data.damage;
	shot->shot_info.backtrack_ticks = backtrack_ticks;
	shot->shot_info.aim_point = final_target.data.point.point;

	g_ctx.globals.aimbot_working = true;
	g_ctx.globals.revolver_working = false;
	g_ctx.globals.last_aimbot_shot = m_globals()->m_tickcount;

	if (g_ctx.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks("aim_fire"))
			current.func();

}

static std::vector<std::tuple<float, float, float>> pre_computed_seeds = {};

void aim::build_seed_table()
{
	if (!pre_computed_seeds.empty()) return;

	for (auto i = 0; i < 128; i++)
	{
		math::random_seed(i + 1);

		const auto pi_seed = math::random_float(0.f, M_PI * 2);

		pre_computed_seeds.emplace_back(math::random_float(0.f, 1.f), sin(pi_seed), cos(pi_seed));
	}
}

int aim::hitchance(const Vector& aim_angle, player_t* e, int hitchance, CUserCmd* cmd)
{
	build_seed_table();

	auto weapon = g_ctx.globals.weapon;
	if (!weapon)
		return false;

	auto weapon_data = weapon->get_csweapon_info();
	if (!weapon_data)
		return false;

	Vector fw, rw, uw;
	math::angle_vectors(aim_angle, &fw, &rw, &uw);

	int hits = 0;
	int needed_hits = static_cast<int>(128 * (hitchance / 100.f));

	float weapon_spread = weapon->get_spread();
	float weapon_innacuracy = weapon->get_inaccuracy();

	Vector src = g_ctx.globals.eye_pos;

	for (int i = 0; i < 128; i++) {

		float a = math::random_float(0.f, 1.f);
		float b = math::random_float(0.f, M_PI * 2.f);
		float c = math::random_float(0.f, 1.f);
		float d = math::random_float(0.f, M_PI * 2.f);
		float inaccuracy = a * weapon_innacuracy;
		float spread = c * weapon_spread;

		if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) {
			if (cmd->m_buttons & IN_ATTACK2) {
				a = 1.f - a * a;
				c = 1.f - c * c;
			}
		}

		Vector spread_view((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0);
		Vector direction;

		direction.x = fw.x + (spread_view.x * rw.x) + (spread_view.y * uw.x);
		direction.y = fw.y + (spread_view.x * rw.y) + (spread_view.y * uw.y);
		direction.z = fw.z + (spread_view.x * rw.z) + (spread_view.y * uw.z);
		direction.Normalized();

		Vector viewangles_spread;
		Vector view_forward;

		math::vector_angles(direction, uw, viewangles_spread);
		viewangles_spread.Normalize();
		math::angle_vectors(viewangles_spread, view_forward);

		view_forward.NormalizeInPlace();
		view_forward = src + (direction * weapon_data->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(src, view_forward);
		m_trace()->ClipRayToEntity(ray, MASK_SHOT, e, &tr);

		if (tr.hit_entity == e)
			hits++;

		if ((128 - i + hits) < needed_hits)
			return false;
	}

	if (static_cast<int>((static_cast<float>(hits) / 129) * 100.f) >= hitchance)
		return true;

	return false;
}

static int clip_ray_to_hitbox(const Ray_t& ray, mstudiobbox_t* hitbox, matrix3x4_t& matrix, trace_t& trace)
{
	static auto fn = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 F3 0F 10 42"));

	trace.fraction = 1.0f;
	trace.startsolid = false;

	return reinterpret_cast <int(__fastcall*)(const Ray_t&, mstudiobbox_t*, matrix3x4_t&, trace_t&)> (fn)(ray, hitbox, matrix, trace);
}

bool aim::hitbox_intersection(player_t* e, matrix3x4_t* matrix, int hitbox, const Vector& start, const Vector& end, float* safe)
{
	auto model = e->GetModel();

	if (!model)
		return false;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return false;

	auto studio_set = studio_model->pHitboxSet(e->m_nHitboxSet());

	if (!studio_set)
		return false;

	auto studio_hitbox = studio_set->pHitbox(hitbox);

	if (!studio_hitbox)
		return false;

	trace_t trace;

	Ray_t ray;
	ray.Init(start, end);

	auto intersected = clip_ray_to_hitbox(ray, studio_hitbox, matrix[studio_hitbox->bone], trace) >= 0;

	if (!safe)
		return intersected;

	Vector min, max;

	math::vector_transform(studio_hitbox->bbmin, matrix[studio_hitbox->bone], min);
	math::vector_transform(studio_hitbox->bbmax, matrix[studio_hitbox->bone], max);

	auto center = (min + max) * 0.5f;
	auto distance = center.DistTo(end);

	if (distance > *safe)
		*safe = distance;

	return intersected;
}