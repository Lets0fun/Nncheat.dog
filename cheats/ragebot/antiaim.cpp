// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "antiaim.h"
#include "knifebot.h"
#include "zeusbot.h"
#include "..\sdk\misc\GlobalVars.hpp"
#include "..\misc\fakelag.h"
#include "..\misc\prediction_system.h"
#include "..\misc\misc.h"
#include "..\lagcompensation\local_animations.h"
#include "Memory.h"

auto cfg = c_config::get( );

void antiaim::create_move( CUserCmd* m_pcmd )
{
	auto velocity = g_ctx.local( )->m_vecVelocity( ).Length( ); //-V807

	type = ANTIAIM_STAND;

	if ( condition( m_pcmd ) )
		return;

	if ( ( type == ANTIAIM_LEGIT ? g_cfg.antiaim.desync : g_cfg.antiaim.type[ type ].desync ) && ( type == ANTIAIM_LEGIT ? !g_cfg.antiaim.legit_lby_type : !g_cfg.antiaim.lby_type ) && !c_config::get( )->b[ "rage_quickstop" ] && ( !g_ctx.globals.weapon->is_grenade( ) || g_cfg.esp.on_click && !( m_pcmd->m_buttons & IN_ATTACK ) && !( m_pcmd->m_buttons & IN_ATTACK2 ) ) && engineprediction::get( ).backup_data.velocity.Length2D( ) <= 20.0f ) //-V648
	{
		auto speed = 1.01f;

		if ( m_pcmd->m_buttons & IN_DUCK || g_ctx.globals.fakeducking )
			speed *= 2.94117647f;

		static auto switch_move = false;

		if ( switch_move )
			m_pcmd->m_sidemove += speed;
		else
			m_pcmd->m_sidemove -= speed;

		switch_move = !switch_move;
	}

	if ( type != ANTIAIM_LEGIT )
		m_pcmd->m_viewangles.x = get_pitch( m_pcmd );

	if ( c_config::get( )->i[ "aa_yawjit" ] == 3 )
		m_pcmd->m_viewangles.y = skitter_jitter( m_pcmd );

	else
		m_pcmd->m_viewangles.y = get_yaw( m_pcmd );

	m_pcmd->m_viewangles.z += c_config::get( )->i[ "rollaa_value" ];

}

float antiaim::get_roll( CUserCmd* m_pcmd )
{
	auto roll_val = m_pcmd->m_viewangles.z;
	roll_val = c_config::get( )->i[ "rollaa_value" ];
	return roll_val;
}

float antiaim::skitter_jitter(CUserCmd* m_pcmd)
{
	auto animstate = g_ctx.local()->get_animation_state();

	float v43 = fabsf(math::AngleDiff(m_pcmd->m_viewangles.y, animstate->m_flGoalFeetYaw)) / m_globals()->m_intervalpertick;

	static bool per_tick = false;
	per_tick = !per_tick;

	if (v43 >= 35)
		m_pcmd->m_viewangles.y += v43 * per_tick;
	else if (v43 <= 35)
		m_pcmd->m_viewangles.y += v43 * per_tick;

	return animstate->m_flGoalFeetYaw = math::normalize_yaw(v43);
}


float antiaim::get_pitch(CUserCmd* m_pcmd) {

	if ((c_config::get()->b["0pitchland"] && (g_ctx.local()->get_animation_state()->m_bInHitGroundAnimation && g_ctx.local()->get_animation_state()->m_flHeadHeightOrOffsetFromHittingGroundAnimation)))
		return 0.0f;

	float pitch = 0.f;

	switch (c_config::get()->i["aa_pitch"]) {
	case 1:
		pitch = 88.914012f;
		break;
	case 2:
		pitch = 45.0f;
		break;
	case 3:
		pitch = -88.914012f;
		break;
	case 4: //Fake pitch
		pitch = 88.914012f;

		if (m_globals()->m_tickcount % 20 == 0)
		{
			pitch = -88.914012f;
		}
		else
			pitch = 88.914012f;
		m_pcmd->m_viewangles.x = pitch;

		break;
	case 5:
		pitch = math::random_float(-88.914012f, 88.914012f);
		break;
	case 6:
		pitch = -88.914012f, 88.914012f;
		break;

	}

	//We use this for local animations
	local_pitch = pitch; 
	return pitch;
}


float antiaim::get_yaw(CUserCmd* m_pcmd)
{
	static auto invert_jitter = false;
	static auto should_invert = false;
	static auto force_choke = false;

	// Weird fix, idefk.
	if (g_ctx.send_packet)
		should_invert = true;
	else if (!g_ctx.send_packet && should_invert)
	{
		should_invert = false;
		invert_jitter = !invert_jitter;
	}

	auto max_desync_delta = g_ctx.local()->get_max_desync_delta(); //-V807

	auto yaw = 0.0f;
	auto lby_type = 0;

	if (c_config::get()->auto_check(c_config::get()->i["freestand_key"], c_config::get()->i["freestand_key_style"]))
		koppel_fs(m_pcmd);
	else
		final_manual_side = manual_side;

	auto base_angle = m_pcmd->m_viewangles.y + 0.0f;

	switch (cfg->i["aa_yaw"])
	{
	case 1:
		base_angle = m_pcmd->m_viewangles.y + 0.0f + cfg->i["aa_yaw_val"];
		break;
	case 2:
		base_angle = m_pcmd->m_viewangles.y + math::fast_sin(cfg->i["aa_yawjit"]);
		break;
	case 3:
		base_angle = m_pcmd->m_viewangles.y + cfg->i["aa_yaw_val"];
		break;
	}

	if (c_config::get()->i["aa_yawbase"])
		base_angle = at_targets();

	if (c_config::get()->i["aa_body_yaw"] == 1)
		flip = key_binds::get().get_key_bind_state(16);

	auto yaw_angle = 0.0f;

	switch (c_config::get()->i["aa_yawjit"])
	{
	case 1:
		yaw_angle = invert_jitter ? 0 : static_cast<float>(c_config::get()->i["aa_yaw_jit"]) * 0.7f;
		break;
	case 2:
		yaw_angle = invert_jitter ? static_cast<float>(c_config::get()->i["aa_yaw_jit"]) * -0.9f : static_cast<float>(c_config::get()->i["aa_yaw_jit"]) * 0.9f;
		break;
	}

	desync_angle = 0.0f;

	if (c_config::get()->i["aa_body_yaw"])
	{
		if (c_config::get()->i["aa_body_yaw"] == 2)
			flip = invert_jitter;

		auto desync_delta = max_desync_delta;

		if (type == ANTIAIM_STAND && cfg->i["aa_body_yaw"] != 3 && cfg->i["aa_body_yaw"])
		{
			desync_delta *= 2.0f;
		}
		else
		{
			if (c_config::get()->i["aa_body_yaw_value"] < 0)
				desync_delta = min(desync_delta, static_cast<float>(-c_config::get()->i["aa_body_yaw_value"]));
			else
				desync_delta = min(desync_delta, static_cast<float>(c_config::get()->i["aa_body_yaw_value"]));
		}

		if (!flip)
		{
			desync_delta = -desync_delta;
			max_desync_delta = -max_desync_delta;
		}

		base_angle -= desync_delta;
		desync_angle = desync_delta;
	}

	yaw = base_angle + yaw_angle;

	if (!desync_angle) //-V550
		return yaw;

	if (c_config::get()->auto_check(c_config::get()->i["freestand_key"], c_config::get()->i["freestand_key_style"]) <= KEY_NONE || c_config::get()->auto_check(c_config::get()->i["freestand_key"], c_config::get()->i["freestand_key_style"]) >= KEY_MAX)
		flip = automatic_direction();

	lby_type = cfg->i["aa_body_yaw"];

	static auto sway_counter = 0;

	if (ShouldBreakLowerBody(m_pcmd, 1))
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

		if (sway_counter > 3)
		{
			if (desync_angle > 0.0f)
				yaw -= 180.0f;
			else
				yaw += 180.0f;
		}

		if (sway_counter < 8)
			++sway_counter;
		else
			sway_counter = 0;

		breaking_lby = true;
		force_choke = true;
		g_ctx.send_packet = false;

		return yaw;
	}
	else if (force_choke)
	{
		force_choke = false;
		g_ctx.send_packet = false;

		return yaw;
	}
}

bool antiaim::ShouldBreakLowerBody(CUserCmd* m_pcmd, int lby_type) {
	if (g_ctx.globals.tochargeamount > 0)
		return false;

	if (g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands > 12)
		return false;

	if (!g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands > 14)
	{
		g_ctx.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	auto animstate = g_ctx.local()->get_animation_state(); //-V807

	if (!animstate)
		return false;

	if (animstate->m_velocity > 0.1f || fabs(animstate->flUpVelocity) > 100.0f)
		g_ctx.globals.next_lby_update = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase + 14);
	else
	{
		if (TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) > g_ctx.globals.next_lby_update)
		{
			g_ctx.globals.next_lby_update = 0.0f;
			return true;
		}
	}

	return false;
}

float quick_normalize( float degree, const float min, const float max ) {
	while ( degree < min )
		degree += max - min;
	while ( degree > max )
		degree -= max - min;

	return degree;
}
bool trace_to_exit_short( Vector& point, Vector& dir, const float step_size, float max_distance )
{
	float flDistance = 0;

	while ( flDistance <= max_distance )
	{
		flDistance += step_size;

		point += dir * flDistance;

		if ( ( m_trace( )->GetPointContents( point ) & MASK_SOLID ) == 0 )
		{
			// found first free point
			return true;
		}
	}

	return false;
}

float get_thickness( Vector& start, Vector& end ) {
	Vector dir = end - start;
	Vector step = start;
	dir /= dir.Length( );
	CTraceFilter filter;
	trace_t trace;
	Ray_t ray;
	float thickness = 0;
	while ( true ) {
		ray.Init( step, end );
		m_trace( )->TraceRay( ray, MASK_SOLID, &filter, &trace );

		if ( !trace.DidHit( ) )
			break;

		const Vector lastStep = trace.endpos;
		step = trace.endpos;

		if ( ( end - start ).Length( ) <= ( step - start ).Length( ) )
			break;

		if ( !trace_to_exit_short( step, dir, 5, 90 ) )
			return FLT_MAX;

		thickness += ( step - lastStep ).Length( );
	}
	return thickness;
}


bool antiaim::condition( CUserCmd* m_pcmd, bool dynamic_check )
{
	if ( !m_pcmd )
		return true;

	if ( !g_ctx.available( ) )
		return true;

	if ( !c_config::get( )->b[ "aa_enabled" ] )
		return true;

	if ( !g_ctx.local( )->is_alive( ) ) //-V807
		return true;

	if ( g_ctx.local( )->m_bGunGameImmunity( ) || g_ctx.local( )->m_fFlags( ) & FL_FROZEN )
		return true;

	if ( g_ctx.local( )->get_move_type( ) == MOVETYPE_NOCLIP || g_ctx.local( )->get_move_type( ) == MOVETYPE_LADDER )
		return true;

	if ( g_ctx.globals.aimbot_working )
		return true;

	auto weapon = g_ctx.local( )->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return true;

	if ( m_pcmd->m_buttons & IN_ATTACK && weapon->m_iItemDefinitionIndex( ) != WEAPON_REVOLVER && !weapon->is_non_aim( ) )
		return true;

	auto revolver_shoot = weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && ( m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2 );

	if ( revolver_shoot )
		return true;

	if ( ( m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2 ) && weapon->is_knife( ) )
		return true;

	if ( dynamic_check && freeze_check )
		return true;

	if ( dynamic_check && m_pcmd->m_buttons & IN_USE && !g_cfg.antiaim.antiaim_type )
		return true;

	if ( dynamic_check && weapon->is_grenade( ) && weapon->m_fThrowTime( ) )
		return true;

	return false;
}

float antiaim::at_targets() {
	player_t* target = nullptr;
	auto best_fov = FLT_MAX;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(true))
			continue;

		auto weapon = e->m_hActiveWeapon().Get();

		if (!weapon)
			continue;

		if (weapon->is_non_aim())
			continue;

		Vector angles;
		m_engine()->GetViewAngles(angles);

		auto fov = math::get_fov(angles, math::calculate_angle(g_ctx.globals.eye_pos, e->GetAbsOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}

	auto angle = 180.0f;

	if (manual_side == SIDE_LEFT)
		angle = 90.0f;
	else if (manual_side == SIDE_RIGHT)
		angle = -90.0f;

	if (!target)
		return g_ctx.get_command()->m_viewangles.y + angle;

	return math::calculate_angle(g_ctx.globals.eye_pos, target->GetAbsOrigin()).y + angle;
}

bool antiaim::automatic_direction( )
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	trace_t tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine( )->GetViewAngles( engineViewAngles );
	engineViewAngles.x = 0.0f;

	math::angle_vectors( engineViewAngles, &forward, &right, &up );

	filter.pSkip = g_ctx.local( );
	src3D = g_ctx.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init( src3D + right * 35.0f, dst3D + right * 35.0f );

	g_ctx.globals.autowalling = true;
	m_trace( )->TraceRay( ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr );
	g_ctx.globals.autowalling = false;

	Right = ( tr.endpos - tr.startpos ).Length( );

	ray_left.Init( src3D - right * 35.0f, dst3D - right * 35.0f );

	g_ctx.globals.autowalling = true;
	m_trace( )->TraceRay( ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr );
	g_ctx.globals.autowalling = false;

	Left = ( tr.endpos - tr.startpos ).Length( );

	static auto left_ticks = 0;
	static auto right_ticks = 0;

	if ( Left - Right > 10.0f )
		left_ticks++;
	else
		left_ticks = 0;

	if ( Right - Left > 10.0f )
		right_ticks++;
	else
		right_ticks = 0;

	if ( right_ticks > 10 )
		return true;
	else if ( left_ticks > 10 )
		return false;

	return flip;
}

void antiaim::freestanding( CUserCmd* m_pcmd )
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	trace_t tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine( )->GetViewAngles( engineViewAngles );
	engineViewAngles.x = 0.0f;

	math::angle_vectors( engineViewAngles, &forward, &right, &up );

	filter.pSkip = g_ctx.local( );
	src3D = g_ctx.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init( src3D + right * 35.0f, dst3D + right * 35.0f );

	g_ctx.globals.autowalling = true;
	m_trace( )->TraceRay( ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr );
	g_ctx.globals.autowalling = false;

	Right = ( tr.endpos - tr.startpos ).Length( );

	ray_left.Init( src3D - right * 35.0f, dst3D - right * 35.0f );

	g_ctx.globals.autowalling = true;
	m_trace( )->TraceRay( ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr );
	g_ctx.globals.autowalling = false;

	Left = ( tr.endpos - tr.startpos ).Length( );

	static auto left_ticks = 0;
	static auto right_ticks = 0;
	static auto back_ticks = 0;

	if ( Right - Left > 20.0f )
		left_ticks++;
	else
		left_ticks = 0;

	if ( Left - Right > 20.0f )
		right_ticks++;
	else
		right_ticks = 0;

	if ( fabs( Right - Left ) <= 20.0f )
		back_ticks++;
	else
		back_ticks = 0;

	if ( right_ticks > 10 )
		final_manual_side = SIDE_RIGHT;
	else if ( left_ticks > 10 )
		final_manual_side = SIDE_LEFT;
	else if ( back_ticks > 10 )
		final_manual_side = SIDE_BACK;
}

void antiaim::koppel_fs( CUserCmd* m_pcmd )
{
	std::vector<angle_data> points;

	if ( !c_config::get( )->auto_check( c_config::get( )->i[ "freestand_key" ], c_config::get( )->i[ "freestand_key_style" ] ) )
		return;

	const auto local = g_ctx.local( );

	const auto local_position = local->m_angEyeAngles( );
	std::vector<float> scanned = {};

	for ( auto i = 0; i <= m_globals( )->m_maxclients; i++ )
	{
		auto p_entity = dynamic_cast< player_t* >( m_entitylist( )->GetClientEntity( i ) );
		if ( p_entity == nullptr ) continue;
		if ( p_entity == local ) continue;
		if ( !p_entity->is_alive( ) ) continue;
		if ( p_entity->m_iTeamNum( ) == local->m_iTeamNum( ) ) continue;
		if ( p_entity->IsDormant( ) ) continue;
		if ( !p_entity->is_player( ) ) continue;

		const auto view = math::calculate_angle( local_position, p_entity->m_angEyeAngles( ) );

		std::vector<angle_data> angs;

		for ( auto y = 1; y < 4; y++ )
		{
			auto ang = quick_normalize( ( y * 90 ) + view.y, -180.f, 180.f );
			auto found = false; // check if we already have a similar angle

			for ( auto i2 : scanned )
				if ( abs( quick_normalize( i2 - ang, -180.f, 180.f ) ) < 20.f )
					found = true;

			if ( found )
				continue;

			points.emplace_back( ang, -1.f );
			scanned.push_back( ang );
		}
		//points.push_back(yaw(view.y, angs)); // base yaws and angle data (base yaw needed for lby breaking etc)
	}

	for ( auto i = 0; i <= m_globals( )->m_maxclients; i++ )
	{
		auto p_entity = dynamic_cast< player_t* >( m_entitylist( )->GetClientEntity( i ) );
		if ( p_entity == nullptr ) continue;
		if ( p_entity == local ) continue;
		if ( !p_entity->is_alive( ) ) continue;
		if ( p_entity->m_iTeamNum( ) == local->m_iTeamNum( ) ) continue;
		if ( p_entity->IsDormant( ) ) continue;
		if ( !p_entity->is_player( ) ) continue;

		auto found = false;
		auto points_copy = points; // copy data so that we compair it to the original later to find the lowest thickness
		auto enemy_eyes = p_entity->m_angEyeAngles( );

		for ( auto& z : points_copy ) // now we get the thickness for all of the data
		{
			const Vector tmp( 10, z.angle, 0.0f );
			Vector head;
			math::AngleVectors( tmp, &head );
			head *= ( ( 16.0f + 3.0f ) + ( ( 16.0f + 3.0f ) * sin( DEG2RAD( 10.0f ) ) ) ) + 7.0f;
			head += local_position;
			const auto local_thickness = get_thickness( head, enemy_eyes ); // i really need my source for this bit, i forgot how it works entirely Autowall :: GetThickness1 (head, hacks.m_local_player, p_entity);
			z.thickness = local_thickness;

			if ( local_thickness != 0 ) // if theres a thickness of 0 dont use this data
			{
				found = true;
			}
		}

		if ( !found ) // dont use
			continue;

		for ( auto z = 0; points_copy.size( ) > z; z++ )
			if ( points_copy[ z ].thickness < points[ z ].thickness || points[ z ].thickness == -1 ) // find the lowest thickness so that we can hide our head best for all entities
				points[ z ].thickness = points_copy[ z ].thickness;

	}
	float best = 0;
	for ( auto& i : points )
		if ( ( i.thickness > best || i.thickness == -1 ) && i.thickness != 0 ) // find the best hiding spot (highest thickness)
		{
			best = i.thickness;
			m_pcmd->m_viewangles.y = i.angle;
			auto ret = true;
		}
}