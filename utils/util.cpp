// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "util.hpp"
#include "../cheats/ragebot/aim.h"
#include "..\cheats\visuals\player_esp.h"
#include "..\cheats\lagcompensation\animation_system.h"
#include "..\cheats\misc\misc.h"
#include <thread>

#define INRANGE(x, a, b) (x >= a && x <= b)  //-V1003
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0)) //-V1003
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

// https://gamesense.pub/fl1pp.1/l3g5nd

namespace util
{
	int epoch_time()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	uintptr_t find_pattern(const char* module_name, const char* pattern, const char* mask)
	{
		MODULEINFO module_info = {};
		K32GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module_name), &module_info, sizeof(MODULEINFO));
		const auto address = reinterpret_cast<std::uint8_t*>(module_info.lpBaseOfDll);
		const auto size = module_info.SizeOfImage;
		std::vector < std::pair < std::uint8_t, bool>> signature;
		for (auto i = 0u; mask[i]; i++)
			signature.emplace_back(std::make_pair(pattern[i], mask[i] == 'x'));
		auto ret = std::search(address, address + size, signature.begin(), signature.end(),
			[](std::uint8_t curr, std::pair<std::uint8_t, bool> curr_pattern)
			{
				return (!curr_pattern.second) || curr == curr_pattern.first;
			});
		return ret == address + size ? 0 : std::uintptr_t(ret);
	}

	uint64_t FindSignature(const char* szModule, const char* szSignature)
	{
		MODULEINFO modInfo;
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(szModule), &modInfo, sizeof(MODULEINFO));

		uintptr_t startAddress = (DWORD)modInfo.lpBaseOfDll; //-V101 //-V220
		uintptr_t endAddress = startAddress + modInfo.SizeOfImage;

		const char* pat = szSignature;
		uintptr_t firstMatch = 0;

		for (auto pCur = startAddress; pCur < endAddress; pCur++)
		{
			if (!*pat)
				return firstMatch;

			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GETBYTE(pat))
			{
				if (!firstMatch)
					firstMatch = pCur;

				if (!pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;
				else
					pat += 2;
			}
			else
			{
				pat = szSignature;
				firstMatch = 0;
			}
		}

		return 0;
	}

	int RandomInt( int min, int max )
	{
		typedef int( *RandomInt_t )( int, int );
		static RandomInt_t m_RandomInt = ( RandomInt_t )GetProcAddress( GetModuleHandle( "vstdlib.dll" ), "RandomInt" );
		return m_RandomInt( min, max );
	}

	bool visible(const Vector& start, const Vector& end, entity_t* entity, player_t* from)
	{
		trace_t trace;

		Ray_t ray;
		ray.Init(start, end);

		CTraceFilter filter;
		filter.pSkip = from;

		g_ctx.globals.autowalling = true;
		m_trace()->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);
		g_ctx.globals.autowalling = false;

		return trace.hit_entity == entity || trace.fraction == 1.0f; //-V550
	}

	bool is_button_down(int code)
	{
		if (code <= KEY_NONE || code >= KEY_MAX)
			return false;

		if (!m_engine()->IsActiveApp())
			return false;

		if (m_engine()->Con_IsVisible())
			return false;

		static auto cl_mouseenable = m_cvar()->FindVar(crypt_str("cl_mouseenable"));

		if (!cl_mouseenable->GetBool())
			return false;

		return m_inputsys()->IsButtonDown((ButtonCode_t)code);
	}

	void movement_fix(Vector& wish_angle, CUserCmd* m_pcmd)
	{
		Vector PureForward, PureRight, PureUp, CurrForward, CurrRight, CurrUp;
		math::angle_vectors( wish_angle, &PureForward, &PureRight, &PureUp );
		math::angle_vectors( m_pcmd->m_viewangles, &CurrForward, &CurrRight, &CurrUp );

		PureForward[ 2 ] = PureRight[ 2 ] = CurrForward[ 2 ] = CurrRight[ 2 ] = 0.f;

		auto VectorNormalize = [ ] ( Vector& vec )->float {
			float radius = sqrtf( vec.x * vec.x + vec.y * vec.y + vec.z * vec.z );
			float iradius = 1.f / ( radius + FLT_EPSILON );

			vec.x *= iradius;
			vec.y *= iradius;
			vec.z *= iradius;

			return radius;
		};
		VectorNormalize( PureForward );
		VectorNormalize( PureRight );
		VectorNormalize( CurrForward );
		VectorNormalize( CurrRight );
		Vector PureWishDir;
		for ( auto i = 0u; i < 2; i++ )
			PureWishDir[ i ] = PureForward[ i ] * m_pcmd->m_forwardmove + PureRight[ i ] * m_pcmd->m_sidemove;
		PureWishDir[ 2 ] = 0.f;

		Vector CurrWishDir;
		for ( auto i = 0u; i < 2; i++ )
			CurrWishDir[ i ] = CurrForward[ i ] * m_pcmd->m_forwardmove + CurrRight[ i ] * m_pcmd->m_sidemove;
		CurrWishDir[ 2 ] = 0.f;

		if ( PureWishDir != CurrWishDir ) {
			m_pcmd->m_forwardmove = ( PureWishDir.x * CurrRight.y - CurrRight.x * PureWishDir.y ) / ( CurrRight.y * CurrForward.x - CurrRight.x * CurrForward.y );
			m_pcmd->m_sidemove = ( PureWishDir.y * CurrForward.x - CurrForward.y * PureWishDir.x ) / ( CurrRight.y * CurrForward.x - CurrRight.x * CurrForward.y );
		}
	}

	unsigned int find_in_datamap(datamap_t* map, const char* name)
	{
		while (map)
		{
			for (auto i = 0; i < map->dataNumFields; ++i)
			{
				if (!map->dataDesc[i].fieldName)
					continue;

				if (!strcmp(name, map->dataDesc[i].fieldName))
					return map->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

				if (map->dataDesc[i].fieldType == FIELD_EMBEDDED)
				{
					if (map->dataDesc[i].td)
					{
						unsigned int offset;

						if (offset = find_in_datamap(map->dataDesc[i].td, name))
							return offset;
					}
				}
			}

			map = map->baseMap;
		}

		return 0;
	}

	bool get_bbox(entity_t* e, Box& box, bool player_esp)
	{
		auto collideable = e->GetCollideable();
		auto m_rgflCoordinateFrame = e->m_rgflCoordinateFrame();

		auto min = collideable->OBBMins();
		auto max = collideable->OBBMaxs();

		Vector points[8] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector pointsTransformed[8];

		for (auto i = 0; i < 8; i++)
			math::vector_transform(points[i], m_rgflCoordinateFrame, pointsTransformed[i]);

		Vector pos = e->GetAbsOrigin();
		Vector flb;
		Vector brt;
		Vector blb;
		Vector frt;
		Vector frb;
		Vector brb;
		Vector blt;
		Vector flt;

		auto bFlb = math::world_to_screen(pointsTransformed[3], flb);
		auto bBrt = math::world_to_screen(pointsTransformed[5], brt);
		auto bBlb = math::world_to_screen(pointsTransformed[0], blb);
		auto bFrt = math::world_to_screen(pointsTransformed[4], frt);
		auto bFrb = math::world_to_screen(pointsTransformed[2], frb);
		auto bBrb = math::world_to_screen(pointsTransformed[1], brb);
		auto bBlt = math::world_to_screen(pointsTransformed[6], blt);
		auto bFlt = math::world_to_screen(pointsTransformed[7], flt);

		if (!bFlb && !bBrt && !bBlb && !bFrt && !bFrb && !bBrb && !bBlt && !bFlt)
			return false;

		Vector arr[8] =
		{
			flb,
			brt,
			blb,
			frt,
			frb,
			brb,
			blt,
			flt
		};

		auto left = flb.x;
		auto top = flb.y;
		auto right = flb.x;
		auto bottom = flb.y;

		for (auto i = 1; i < 8; i++)
		{
			if (left > arr[i].x)
				left = arr[i].x;
			if (top < arr[i].y)
				top = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (bottom > arr[i].y)
				bottom = arr[i].y;
		}

		box.x = left;
		box.y = bottom;
		box.w = right - left;
		box.h = top - bottom;

		return true;
	}

	void trace_line(Vector& start, Vector& end, unsigned int mask, CTraceFilter* filter, CGameTrace* tr)
	{
		Ray_t ray;
		ray.Init(start, end);

		m_trace()->TraceRay(ray, mask, filter, tr);
	}

	void clip_trace_to_players(IClientEntity* e, const Vector& start, const Vector& end, unsigned int mask, CTraceFilter* filter, CGameTrace* tr)
	{
		Vector mins = e->GetCollideable()->OBBMins(), maxs = e->GetCollideable()->OBBMaxs();

		Vector dir(end - start);
		dir.Normalize();

		Vector
			center = (maxs + mins) / 2,
			pos(center + e->GetAbsOrigin());

		Vector to = pos - start;
		float range_along = dir.Dot(to);

		float range;
		if (range_along < 0.f)
			range = -to.Length();

		else if (range_along > dir.Length())
			range = -(pos - end).Length();

		else {
			auto ray(pos - (dir * range_along + start));
			range = ray.Length();
		}

		if (range <= 60.f) {
			trace_t trace;

			Ray_t ray;
			ray.Init(start, end);

			m_trace()->ClipRayToEntity(ray, mask, e, &trace);

			if (tr->fraction > trace.fraction)
				*tr = trace;
		}
	}

	void RotateMovement(CUserCmd* cmd, float yaw)
	{
		Vector viewangles;
		m_engine()->GetViewAngles(viewangles);

		float rotation = DEG2RAD(viewangles.y - yaw);

		float cos_rot = cos(rotation);
		float sin_rot = sin(rotation);

		float new_forwardmove = cos_rot * cmd->m_forwardmove - sin_rot * cmd->m_sidemove;
		float new_sidemove = sin_rot * cmd->m_forwardmove + cos_rot * cmd->m_sidemove;

		cmd->m_forwardmove = new_forwardmove;
		cmd->m_sidemove = new_sidemove;
	}

	void color_modulate(float color[3], IMaterial* material)
	{
		auto found = false;
		auto var = material->FindVar(crypt_str("$envmaptint"), &found);

		if (found)
			var->set_vec_value(color[0], color[1], color[2]);

		m_renderview()->SetColorModulation(color[0], color[1], color[2]);
	}

	bool get_backtrack_matrix(player_t* e, matrix3x4_t* matrix)
	{
		if ((!c_config::get()->b["rage_enabled"] && !c_config::get()->auto_check(c_config::get()->i["rage_key_enabled"], c_config::get()->i["rage_key_enabled_st"])))
			return false;

		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return false;

		auto i = e->EntIndex();

		if (i < 1 || i > 64)
			return false;

		auto records = &player_records[i]; //-V826

		if (records->size() < 2)
			return false;

		for (auto record = records->rbegin(); record != records->rend(); ++record)
		{
			if (!record->valid())
				continue;

			if (record->origin.DistTo(e->GetAbsOrigin()) < 1.0f)
				return false;

			auto curtime = m_globals()->m_curtime;
			auto range = 0.2f;

			if (g_ctx.local()->is_alive())
				curtime = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);

			auto next_record = record + 1;
			auto end = next_record == records->rend();

			auto next = end ? e->GetAbsOrigin() : next_record->origin;
			auto time_next = end ? e->m_flSimulationTime() : next_record->simulation_time;

			auto correct = nci->GetLatency(FLOW_OUTGOING) + nci->GetLatency(FLOW_INCOMING) + util::get_interpolation();
			auto time_delta = time_next - record->simulation_time;

			auto add = end ? range : time_delta;
			auto deadtime = record->simulation_time + correct + add;
			auto delta = deadtime - curtime;

			auto mul = 1.0f / add;
			auto lerp = math::lerp(next, record->origin, math::clamp(delta * mul, 0.0f, 1.0f));

			matrix3x4_t result[MAXSTUDIOBONES];
			memcpy(result, record->matrixes_data.main, MAXSTUDIOBONES * sizeof(matrix3x4_t));

			for (auto j = 0; j < MAXSTUDIOBONES; j++)
			{
				auto matrix_delta = math::matrix_get_origin(record->matrixes_data.main[j]) - record->origin;
				math::matrix_set_origin(matrix_delta + lerp, result[j]);
			}

			memcpy(matrix, result, MAXSTUDIOBONES * sizeof(matrix3x4_t));
			return true;
		}

		return false;
	}

	void create_state(c_baseplayeranimationstate* state, player_t* e)
	{
		using Fn = void(__thiscall*)(c_baseplayeranimationstate*, player_t*);
		static auto fn = reinterpret_cast <Fn> (util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 56 8B F1 B9 ? ? ? ? C7 46")));

		fn(state, e);
	}

	void update_state(c_baseplayeranimationstate* state, const Vector& angles)
	{
		using Fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
		static auto fn = reinterpret_cast <Fn> (util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24")));

		fn(state, nullptr, 0.0f, angles[1], angles[0], nullptr);
	}

	void reset_state(c_baseplayeranimationstate* state)
	{
		using Fn = void(__thiscall*)(c_baseplayeranimationstate*);
		static auto fn = reinterpret_cast <Fn> (util::FindSignature(crypt_str("client.dll"), crypt_str("56 6A 01 68 ? ? ? ? 8B F1")));

		fn(state);
	}

	void autopeek_fix(CUserCmd* cmd)
	{
		bool can_peek = !g_ctx.globals.weapon->is_non_aim() && c_config::get()->b["rage_quick_peek_assist"] && c_config::get()->auto_check(c_config::get()->i["rage_quickpeek_enabled"], c_config::get()->i["rage_quickpeek_enabled_style"]);

		if (!can_peek)
			return;

		auto velocity = g_ctx.local()->m_vecVelocity();

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

			cmd->m_forwardmove = negative_forward_direction.x;
			cmd->m_sidemove = negative_side_direction.y;
		}
	}

	void copy_command(CUserCmd* cmd, int tickbase_shift)
	{
		bool can_peek = !g_ctx.globals.weapon->is_non_aim() && c_config::get()->b["rage_quick_peek_assist"] && c_config::get()->auto_check(c_config::get()->i["rage_quickpeek_enabled"], c_config::get()->i["rage_quickpeek_enabled_style"]);

		auto fast_stop = [&](CUserCmd* cur_cmd)
		{
			if (!can_peek)
				return;

			auto velocity = g_ctx.local()->m_vecVelocity();

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

				cur_cmd->m_forwardmove = negative_forward_direction.x;
				cur_cmd->m_sidemove = negative_side_direction.y;
			}
		};

		Vector wish_angle;
		m_engine()->GetViewAngles(wish_angle);

		util::movement_fix(wish_angle, cmd);

		fast_stop(cmd);

		auto commands_to_add = 0;

		do
		{
			auto sequence_number = commands_to_add + cmd->m_command_number;

			auto command = m_input()->GetUserCmd(sequence_number);
			auto verified_command = m_input()->GetVerifiedUserCmd(sequence_number);

			memcpy(command, cmd, sizeof(CUserCmd)); //-V598

			if (command->m_tickcount != INT_MAX && m_clientstate()->iDeltaTick > 0)
				m_prediction()->Update(m_clientstate()->iDeltaTick,
					true,
					m_clientstate()->nLastCommandAck,
					m_clientstate()->nLastOutgoingCommand + m_clientstate()->iChokedCommands);

			command->m_command_number = sequence_number;
			command->m_predicted = command->m_tickcount != INT_MAX;

			++m_clientstate()->iChokedCommands; //-V807

			if (m_clientstate()->pNetChannel)
			{
				++m_clientstate()->pNetChannel->m_nChokedPackets;
				++m_clientstate()->pNetChannel->m_nOutSequenceNr;
			}

			math::normalize_angles(command->m_viewangles);

			memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd)); //-V598
			verified_command->m_crc = command->GetChecksum();

			++commands_to_add;
		} while (commands_to_add != tickbase_shift);

		*(bool*)((uintptr_t)m_prediction() + 0x24) = true;
		*(int*)((uintptr_t)m_prediction() + 0x1C) = 0;
	}

	float get_interpolation()
	{
		float updaterate = m_cvar()->FindVar("cl_updaterate")->GetFloat();
		auto minupdate = m_cvar()->FindVar("sv_minupdaterate");
		auto maxupdate = m_cvar()->FindVar("sv_maxupdaterate");

		if (minupdate && maxupdate)
			updaterate = maxupdate->GetFloat();

		float ratio = m_cvar()->FindVar("cl_interp_ratio")->GetFloat();

		if (ratio == 0)
			ratio = 1.0f;

		float lerp = m_cvar()->FindVar("cl_interp")->GetFloat();
		auto cmin = m_cvar()->FindVar("sv_client_min_interp_ratio");
		auto cmax = m_cvar()->FindVar("sv_client_max_interp_ratio");

		if (cmin && cmax && cmin->GetFloat() != 1)
			ratio = math::clamp(ratio, cmin->GetFloat(), cmax->GetFloat());

		return max(lerp, ratio / updaterate);
	}
}