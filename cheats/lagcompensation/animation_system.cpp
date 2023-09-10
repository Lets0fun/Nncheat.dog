// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\misc\misc.h"
#include "..\misc\logs.h"

std::deque <adjust_data> player_records[65];
void lagcompensation::fsn(ClientFrameStage_t stage)
{
	if (stage != FRAME_NET_UPDATE_END)
		return;

	if (!c_config::get()->b["rage_enabled"])
		return;

	for (auto i = 1; i < m_globals()->m_maxclients; ++i)
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (e == g_ctx.local())
			continue;

		if (!valid(i, e))
			continue;

		if (player_records[i].empty() || (!player_records[i].empty() && e->m_flSimulationTime() != e->m_flOldSimulationTime()))
		{
			if (!player_records[i].empty() && (e->m_vecOrigin() - player_records[i].front().origin).LengthSqr() > 4096.0f)
				for (auto& record : player_records[i])
					record.invalid = true;

			player_records[i].emplace_front(adjust_data());
			update_player_animations(e);

			if (player_records[i].size() > 32)
				player_records[i].pop_back();


			// ��������� � �������. �� ��������� ������ ������� ���� ���: e->m_flSimulationTime() != e->m_flOldSimulationTime()
			// ��� ������, ��� ����� ��������� �����������, �� ��� �������� ������������ � �������� ���������.
			// ����� ��� ������������ � ������ ����� - ����� �������� !player_records[i].empty(), � ����� ��� ����� ������������ ����� 
			// ������ � ���������� �� �����.

		}
	}
}

bool lagcompensation::valid(int i, player_t* e)
{

	if (!c_config::get()->b["rage_enabled"] || !e->valid(false))
	{
		if (!e->is_alive())
		{
			is_dormant[i] = false;
			player_resolver[i].reset();

			g_ctx.globals.fired_shots[i] = 0;
			g_ctx.globals.missed_shots[i] = 0;
		}
		else if (e->IsDormant())
			is_dormant[i] = true;

		player_records[i].clear();
		return false;
	}

	return true;
}

bool IsNearEqual(float v1, float v2, float Tolerance)
{
	return std::abs(v1 - v2) <= std::abs(Tolerance);
};

void lagcompensation::ent_use_jitter(player_t* player, int* new_side, adjust_data* player_record) {

	if (!player->is_alive())
		return;

	if (!player->valid(false, false))
		return;

	if (player->IsDormant())
		return;

	static float LastAngle[64];
	static int LastBrute[64];
	static bool Switch[64];
	static float LastUpdateTime[64];
	AnimationLayer layers[13];

	memcpy(layers, player->get_animlayers(), player->animlayer_count() * sizeof(AnimationLayer));

	int i = player->EntIndex();
	auto animstate = player->get_animation_state();
	float speed = player->m_vecVelocity().Length2D();
	auto delta = math::AngleDiff(animstate->m_flGoalFeetYaw, animstate->m_flEyeYaw);
	float CurrentAngle = player->m_angEyeAngles().y;
	float goalfeetyaw = animstate->m_flGoalFeetYaw;

	// static jitter fix

	if (layers[3].m_flWeight <= 0.1f)
	{
		if (IsNearEqual(CurrentAngle, LastAngle[i], 50.f))
		{
			Switch[i] = !Switch[i];
			LastAngle[i] = CurrentAngle;
			*new_side = Switch[i] ? -1 : 1;
			LastBrute[i] = *new_side;
			LastUpdateTime[i] = m_globals()->m_curtime;
		}
		else {
			if (fabsf(LastUpdateTime[i] - m_globals()->m_curtime >= TICKS_TO_TIME(17))
				|| player->m_flSimulationTime() != player->m_flOldSimulationTime()) {
				LastAngle[i] = CurrentAngle;
			}
			*new_side = LastBrute[i];
		}
	}
}

void lagcompensation::extrapolate(player_t* player, Vector& origin, Vector& velocity, int& flags, bool on_ground)
{
	Vector start, end, normal;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;
	Ray_t Ray;

	// define trace start.
	start = origin;

	// move trace end one tick into the future using predicted velocity.
	end = start + (velocity * m_globals()->m_intervalpertick);

	// trace.
	Ray.Init(start, end, player->m_vecMins(), player->m_vecMaxs());

	const auto mask = (MASK_PLAYERSOLID & ~CONTENTS_MONSTER);

	m_trace()->TraceRay(Ray, mask, &filter, &trace);

	// we hit shit
	// we need to fix hit.
	if (trace.fraction != 1.f)
	{
		// fix sliding on planes.
		for (int i{}; i < 2; ++i) {
			velocity -= trace.plane.normal * velocity.Dot(trace.plane.normal);

			float adjust = velocity.Dot(trace.plane.normal);
			if (adjust < 0.f)
				velocity -= (trace.plane.normal * adjust);

			start = trace.endpos;
			end = start + (velocity * (m_globals()->m_intervalpertick * (1.f - trace.fraction)));

			Ray_t two_ray;
			two_ray.Init(start, end, player->m_vecMins(), player->m_vecMaxs());
			m_trace()->TraceRay(two_ray, mask, &filter, &trace);
			if (trace.fraction == 1.f)
				break;
		}
	}

	// set new final origin.
	start = end = origin = trace.endpos;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	Ray_t ThreeRay;
	ThreeRay.Init(start, end, player->m_vecMins(), player->m_vecMaxs());

	// strip onground flag.
	m_trace()->TraceRay(ThreeRay, mask, &filter, &trace);
	flags &= ~FL_ONGROUND;

	// add back onground flag if we are onground.
	if (trace.fraction != 1.f && trace.plane.normal.z > 0.7f)
		flags != FL_ONGROUND;
}

#define CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX 0.8f	
#define CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX_SCOPED 4.2f  

inline Vector Approach(Vector target, Vector value, float speed)
{
	Vector diff = (target - value);
	float delta = diff.Length();

	if (delta > speed)
		value += diff.Normalized() * speed;
	else if (delta < -speed)
		value -= diff.Normalized() * speed;
	else
		value = target;

	return value;
}
inline float Approach(float target, float value, float speed)
{
	float delta = target - value;

#if defined(_X360) || defined( _PS3 ) // use conditional move for speed on 360

	return fsel(delta - speed,	// delta >= speed ?
		value + speed,	// if delta == speed, then value + speed == value + delta == target  
		fsel((-speed) - delta, // delta <= -speed
			value - speed,
			target)
	);  // delta < speed && delta > -speed

#else

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;

#endif
}

struct aimmatrix_transition_t
{
	float	m_flDurationStateHasBeenValid;
	float	m_flDurationStateHasBeenInValid;
	float	m_flHowLongToWaitUntilTransitionCanBlendIn;
	float	m_flHowLongToWaitUntilTransitionCanBlendOut;
	float	m_flBlendValue;

	void UpdateTransitionState(bool bStateShouldBeValid, float flTimeInterval, float flSpeed)
	{
		if (bStateShouldBeValid)
		{
			m_flDurationStateHasBeenInValid = 0;
			m_flDurationStateHasBeenValid += flTimeInterval;
			if (m_flDurationStateHasBeenValid >= m_flHowLongToWaitUntilTransitionCanBlendIn)
			{
				m_flBlendValue = Approach(1, m_flBlendValue, flSpeed);
			}
		}
		else
		{
			m_flDurationStateHasBeenValid = 0;
			m_flDurationStateHasBeenInValid += flTimeInterval;
			if (m_flDurationStateHasBeenInValid >= m_flHowLongToWaitUntilTransitionCanBlendOut)
			{
				m_flBlendValue = Approach(0, m_flBlendValue, flSpeed);
			}
		}
	}

	void Init(void)
	{
		m_flDurationStateHasBeenValid = 0;
		m_flDurationStateHasBeenInValid = 0;
		m_flHowLongToWaitUntilTransitionCanBlendIn = 0.3f;
		m_flHowLongToWaitUntilTransitionCanBlendOut = 0.3f;
		m_flBlendValue = 0;
	}

	aimmatrix_transition_t()
	{
		Init();
	}
};


void lagcompensation::OnRestore(player_t* e, adjust_data* player_record)
{
	// ��� ����� ������ �� ��� ������� ��� �/�
}



void lagcompensation::SetUpAimMatrix(player_t* e)
{
	auto animstate = e->get_animation_state();
	float m_flLastUpdateIncrement = 0.f;
	float m_flSpeedAsPortionOfWalkTopSpeed = 0.f;
	float m_flSpeedAsPortionOfRunTopSpeed = 0.f;
	aimmatrix_transition_t	m_tStandWalkAim;
	aimmatrix_transition_t	m_tStandRunAim;
	aimmatrix_transition_t	m_tCrouchWalkAim;
	float m_flSpeedAsPortionOfCrouchTopSpeed = 0.f;

	if (animstate->m_fDuckAmount <= 0 || animstate->m_fDuckAmount >= 1) // only transition aim pose when fully ducked or fully standing
	{
		bool bPlayerIsWalking = (e && e->m_bIsWalking());
		bool bPlayerIsScoped = (e && e->m_bIsScoped());

		float flTransitionSpeed = m_flLastUpdateIncrement * (bPlayerIsScoped ? CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX_SCOPED : CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX);

		if (bPlayerIsScoped) // hacky: just tell all the transitions they've been invalid too long so all transitions clear as soon as the player starts scoping
		{
			m_tStandWalkAim.m_flDurationStateHasBeenInValid = m_tStandWalkAim.m_flHowLongToWaitUntilTransitionCanBlendOut;
			m_tStandRunAim.m_flDurationStateHasBeenInValid = m_tStandRunAim.m_flHowLongToWaitUntilTransitionCanBlendOut;
			m_tCrouchWalkAim.m_flDurationStateHasBeenInValid = m_tCrouchWalkAim.m_flHowLongToWaitUntilTransitionCanBlendOut;
		}

		m_tStandWalkAim.UpdateTransitionState(bPlayerIsWalking && !bPlayerIsScoped && m_flSpeedAsPortionOfWalkTopSpeed > 0.7f && m_flSpeedAsPortionOfRunTopSpeed < 0.7,
			m_flLastUpdateIncrement, flTransitionSpeed);

		m_tStandRunAim.UpdateTransitionState(!bPlayerIsScoped && m_flSpeedAsPortionOfRunTopSpeed >= 0.7,
			m_flLastUpdateIncrement, flTransitionSpeed);

		m_tCrouchWalkAim.UpdateTransitionState(!bPlayerIsScoped && m_flSpeedAsPortionOfCrouchTopSpeed >= 0.5,
			m_flLastUpdateIncrement, flTransitionSpeed);
	}

	// Set aims to zero weight if they're underneath aims with 100% weight, for animation perf optimization.
	// Also set aims to full weight if their overlapping aims aren't enough to cover them, because cross-fades don't sum to 100% weight.

	float flStandIdleWeight = 1;
	float flStandWalkWeight = m_tStandWalkAim.m_flBlendValue;
	float flStandRunWeight = m_tStandRunAim.m_flBlendValue;
	float flCrouchIdleWeight = 1;
	float flCrouchWalkWeight = m_tCrouchWalkAim.m_flBlendValue;

	if (flStandWalkWeight >= 1)
		flStandIdleWeight = 0;

	if (flStandRunWeight >= 1)
	{
		flStandIdleWeight = 0;
		flStandWalkWeight = 0;
	}

	if (flCrouchWalkWeight >= 1)
		flCrouchIdleWeight = 0;

	if (animstate->m_fDuckAmount >= 1)
	{
		flStandIdleWeight = 0;
		flStandWalkWeight = 0;
		flStandRunWeight = 0;
	}
	else if (animstate->m_fDuckAmount <= 0)
	{
		flCrouchIdleWeight = 0;
		flCrouchWalkWeight = 0;
	}

	float flOneMinusDuckAmount = 1.0f - animstate->m_fDuckAmount;

	flCrouchIdleWeight *= animstate->m_fDuckAmount;
	flCrouchWalkWeight *= animstate->m_fDuckAmount;
	flStandWalkWeight *= flOneMinusDuckAmount;
	flStandRunWeight *= flOneMinusDuckAmount;

	// make sure idle is present underneath cross-fades
	if (flCrouchIdleWeight < 1 && flCrouchWalkWeight < 1 && flStandWalkWeight < 1 && flStandRunWeight < 1)
		flStandIdleWeight = 1;

}


void lagcompensation::setupvelocity(player_t* e, adjust_data* record)
{
	auto animstate = e->get_animation_state();

	AnimationLayer animlayers[13];

	memcpy(animlayers, e->get_animlayers(), 13 * sizeof(AnimationLayer));


	auto m_pState = e->get_animation_state();
	static const float CS_PLAYER_SPEED_RUN = 260.0f;

	// TODO: Find these members in the actual animstate struct
	auto m_flLastUpdateIncrement = *(float*)((DWORD)m_pState + 0x74);
	float m_flFootYaw = m_pState->m_flGoalFeetYaw;
	float m_flMoveYaw = m_pState->m_flCurrentTorsoYaw;
	auto m_vecVelocityNormalizedNonZero = *(Vector*)((DWORD)m_pState + 0xE0);
	auto m_flInAirSmoothValue = *(float*)((DWORD)m_pState + 0x124);
	AnimationLayer* m_AnimationData;

	char m_szDestination[64];

	int m_nMoveSequence = e->LookupAttachment(m_szDestination);
	if (m_nMoveSequence == -1)
	{
		m_nMoveSequence = e->LookupAttachment(crypt_str("move"));
	}

	// NOTE:
	// player->get<int>( 0x3984 ) is m_iMoveState

	float m_flYaw = math::AngleDiff((m_pState->m_flCurrentTorsoYaw + m_pState->m_flGoalFeetYaw), 180.f);
	Vector m_angAngle = { 0.0f, m_flYaw, 0.0f };
	Vector m_vecDirection;
	math::angle_vectors(m_angAngle, m_vecDirection);

	float m_flMovementSide = math::dot_product(m_vecVelocityNormalizedNonZero, m_vecDirection);
	if (m_flMovementSide < 0.0f)
		m_flMovementSide = -m_flMovementSide;

	float m_flNewFeetWeight = math::angle_distance(m_flMovementSide, 0.2f) * animlayers->m_flWeight;

	float m_flNewFeetWeightWithAirSmooth = m_flNewFeetWeight * m_flInAirSmoothValue;

	// m_flLayer5Weight looks a bit weird so i decided to name it m_flLayer5_Weight instead.
	float m_flLayer5_Weight = animlayers[5].m_flWeight;

	float m_flNewWeight = 0.55f;
	if (1.0f - m_flLayer5_Weight > 0.55f)
		m_flNewWeight = 1.0f - m_flLayer5_Weight;

	float m_flNewFeetWeightLayerWeight = m_flNewWeight * m_flNewFeetWeightWithAirSmooth;
	float m_flFeetCycleRate = 0.0f;

	float m_flSpeed = std::fmin(e->m_vecVelocity().Length(), 260.f);

	animlayers->m_nSequence = m_nMoveSequence;
	animlayers->m_flWeight = std::clamp(m_flNewFeetWeightLayerWeight, 0.0f, 1.0f);

	if (animstate->m_flFeetSpeedForwardsOrSideWays >= 0.0)
		animstate->m_flFeetSpeedForwardsOrSideWays = fminf(animstate->m_flFeetSpeedForwardsOrSideWays, 1.0);
	else
		animstate->m_flFeetSpeedForwardsOrSideWays = 0.0;

	auto v54 = animstate->m_fDuckAmount;
	auto v55 = ((((*(float*)((uintptr_t)animstate + 0x11C)) * -0.30000001) - 0.19999999) * animstate->m_flFeetSpeedForwardsOrSideWays) + 1.0f;

	if (v54 > 0.0)
	{
		if (animstate->m_flFeetSpeedUnknownForwardOrSideways >= 0.0)
			animstate->m_flFeetSpeedUnknownForwardOrSideways = fminf(animstate->m_flFeetSpeedUnknownForwardOrSideways, 1.0);
		else
			animstate->m_flFeetSpeedUnknownForwardOrSideways = 0.0;

	}

	bool bWasMovingLastUpdate = false;
	bool bJustStartedMovingLastUpdate = false;

	if (e->m_vecVelocity().Length2D() <= 0.0f)
	{
		animstate->m_flTimeSinceStartedMoving = 0.0f;
		bWasMovingLastUpdate = animstate->m_flTimeSinceStoppedMoving <= 0.0f;
		animstate->m_flTimeSinceStoppedMoving += animstate->m_flLastClientSideAnimationUpdateTime;
	}
	else
	{
		animstate->m_flTimeSinceStoppedMoving = 0.0f;
		bJustStartedMovingLastUpdate = animstate->m_flTimeSinceStartedMoving <= 0.0f;
		animstate->m_flTimeSinceStartedMoving = animstate->m_flLastClientSideAnimationUpdateTime + animstate->m_flTimeSinceStartedMoving;
	}
	if (animstate->m_flFeetSpeedUnknownForwardOrSideways < 1.0f)
	{
		if (animstate->m_flFeetSpeedUnknownForwardOrSideways < 0.5f)
		{
			float unknown_velocity = 0.f;
			float velocity = unknown_velocity;
			float delta = animstate->m_flLastClientSideAnimationUpdateTime * 60.0f;
			float new_velocity;
			if ((80.0f - velocity) <= delta)
			{
				if (-delta <= (80.0f - velocity))
					new_velocity = 80.0f;
				else
					new_velocity = velocity - delta;
			}
			else
			{
				new_velocity = velocity + delta;
			}
			unknown_velocity = new_velocity;
		}
	}
}


void lagcompensation::animevent(player_t* e, c_baseplayeranimationstate* state, int order, int activity)
{
	auto v18 = state->m_fDuckAmount > 0.55f;
	auto v15 = state->m_velocity > 0.25f;
	int sequence = 0;
	auto p_layer = &e->get_animlayers()[order];
	switch (activity)
	{
	case 985:
		sequence = (int)v15 + 17;
		if (!v18)
			sequence = (int)v15 + 15;
		goto LABEL_14;
	case 986:
		sequence = 14;
		goto LABEL_14;
	case 987:
		sequence = 13;
		goto LABEL_14;
	case 988:
		sequence = 2 * (int)v15 + 20;
		if (v18)
		{
			sequence = 21;
			if (v15)
				sequence = 19;
		}
		goto LABEL_14;
	case 989:
		sequence = 23;
		if (v18)
			sequence = 24;
	LABEL_14:

		p_layer->m_nSequence = sequence;
		p_layer->m_flWeight = 0;
		p_layer->m_flCycle = 0;
		break;
	default:
		return;
	}
}


void lagcompensation::update_player_animations(player_t* e)
{

	auto animstate = e->get_animation_state();

	if (!animstate)
		return;

	player_info_s player_info;

	if (!m_engine()->GetPlayerInfo(e->EntIndex(), &player_info))
		return;

	auto records = &player_records[e->EntIndex()];

	if (records->empty())
		return;

	adjust_data* previous_record = nullptr;

	if (records->size() >= 2)
		previous_record = &records->at(1);

	auto record = &records->front();

	AnimationLayer animlayers[15];

	memcpy(animlayers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(record->layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));


	auto backup_lower_body_yaw_target = e->m_flLowerBodyYawTarget();
	auto backup_duck_amount = e->m_flDuckAmount();
	auto backup_flags = e->m_fFlags();
	auto backup_eflags = e->m_iEFlags();

	auto backup_curtime = m_globals()->m_curtime;
	auto backup_frametime = m_globals()->m_frametime;
	auto backup_realtime = m_globals()->m_realtime;
	auto backup_framecount = m_globals()->m_framecount;
	auto backup_tickcount = m_globals()->m_tickcount;
	auto backup_interpolation_amount = m_globals()->m_interpolation_amount;

	m_globals()->m_curtime = e->m_flSimulationTime();
	m_globals()->m_frametime = m_globals()->m_intervalpertick;
	auto current_weapon = e->m_hActiveWeapon().Get();

	if (previous_record)
	{
		record->shot = record->last_shot_time > previous_record->simulation_time && record->last_shot_time <= record->simulation_time;

		auto velocity = e->m_vecVelocity();
		auto was_in_air = e->m_fFlags() & FL_ONGROUND && previous_record->flags & FL_ONGROUND;

		auto time_difference = max(m_globals()->m_intervalpertick, e->m_flSimulationTime() - previous_record->simulation_time);
		auto origin_delta = e->m_vecOrigin() - previous_record->origin;

		auto animation_speed = 0.0f;


		if (!origin_delta.IsZero() && TIME_TO_TICKS(time_difference) > 0)
		{
			e->m_vecVelocity() = origin_delta * (1.0f / time_difference);

			if (e->m_fFlags() & FL_ONGROUND && animlayers[11].m_flWeight > 0.0f && animlayers[11].m_flWeight < 1.0f && animlayers[11].m_flCycle > previous_record->layers[11].m_flCycle)
			{
				auto weapon = e->m_hActiveWeapon().Get();

				if (weapon)
				{
					auto max_speed = 260.0f;
					auto weapon_info = e->m_hActiveWeapon().Get()->get_csweapon_info();

					if (weapon_info)
						max_speed = e->m_bIsScoped() ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

					auto modifier = 0.35f * (1.0f - animlayers[11].m_flWeight);

					if (modifier > 0.0f && modifier < 1.0f)
						animation_speed = max_speed * (modifier + 0.55f);
				}
			}

			if (animation_speed > 0.0f)
			{
				animation_speed /= e->m_vecVelocity().Length2D();

				e->m_vecVelocity().x *= animation_speed;
				e->m_vecVelocity().y *= animation_speed;
			}

			if (records->size() >= 3 && time_difference > m_globals()->m_intervalpertick)
			{
				auto previous_velocity = (previous_record->origin - records->at(2).origin) * (1.0f / time_difference);

				if (!previous_velocity.IsZero() && !was_in_air)
				{
					auto current_direction = math::normalize_yaw(RAD2DEG(atan2(e->m_vecVelocity().y, e->m_vecVelocity().x)));
					auto previous_direction = math::normalize_yaw(RAD2DEG(atan2(previous_velocity.y, previous_velocity.x)));

					auto average_direction = current_direction - previous_direction;
					average_direction = DEG2RAD(math::normalize_yaw(current_direction + average_direction * 0.5f));

					auto direction_cos = cos(average_direction);
					auto dirrection_sin = sin(average_direction);

					auto velocity_speed = e->m_vecVelocity().Length2D();
					if (animlayers[6].m_flPlaybackRate == 0.f) {
						e->m_vecVelocity().Zero();
					}
					else
					{
						const auto avg_speed = e->m_vecVelocity().Length2D();
						if (avg_speed != 0.f) {
							const auto weight = animlayers[11].m_flWeight;

							const auto speed_as_portion = 0.55f - (weight - 1.f) * 0.35f;

							const auto avg_speed_modifier = speed_as_portion * (current_weapon ? max(current_weapon->get_csweapon_info()->flMaxPlayerSpeed, 0.001f) : 260.f);

							if (weight >= 1.f && avg_speed > avg_speed_modifier
								|| weight < 1.f && (avg_speed_modifier > avg_speed || weight > 0.f)) {
								e->m_vecVelocity().x /= avg_speed;
								e->m_vecVelocity().y /= avg_speed;

								e->m_vecVelocity() *= avg_speed_modifier;
								e->m_vecVelocity() *= avg_speed_modifier;
							}
						}
					}
				}
			}
			if (!(e->m_fFlags() & FL_ONGROUND))
			{
				static auto sv_gravity = m_cvar()->FindVar(crypt_str("sv_gravity"));

				auto tick = std::clamp(TIME_TO_TICKS(time_difference), 1, 16);
				e->m_vecVelocity().z -= (sv_gravity->GetFloat() * TICKS_TO_TIME(tick) * 0.5f);	// true flight fix
			}
			else
				e->m_vecVelocity().z = 0.0f;
		}
	}
	else
	{
		record->shot = record->last_shot_time == record->simulation_time;
	}


	if (animlayers[6].m_flWeight == 0.0f || animlayers[6].m_flPlaybackRate == 0.0f)
		e->m_vecVelocity().Zero();

	e->m_iEFlags() &= ~EFL_DIRTY_ABSVELOCITY;


	auto activity = e->sequence_activity(animlayers[3].m_nSequence);

	if (animstate->m_flTimeSinceStoppedMoving > 0.1f && e->m_vecVelocity().Length2D() < 0.1f || animlayers[6].m_flWeight <= 0.1f)
		activity = ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING; // 980 stands

	e->m_vecAbsVelocity() = e->m_vecVelocity();
	e->m_bClientSideAnimation() = true;

	if (is_dormant[e->EntIndex()])
	{
		is_dormant[e->EntIndex()] = false;

		if (e->m_fFlags() & FL_ONGROUND)
		{
			animstate->m_bOnGround = true;
			animstate->m_bInHitGroundAnimation = false;
		}

		animstate->time_since_in_air() = 0.0f;
		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y);
	}

	auto updated_animations = false;

	c_baseplayeranimationstate state;
	memcpy(&state, animstate, sizeof(c_baseplayeranimationstate));


	bool old_onground = animstate->m_bOnGround;
	bool new_onladder = !animstate->m_bOnGround && e->get_move_type() == MOVETYPE_LADDER;
	bool new_onground = e->m_fFlags() & FL_ONGROUND;

	bool just_landed;
	if (old_onground != new_onground && new_onground)
		just_landed = true;
	else
		just_landed = false;

	if (animstate->m_bOnGround)
	{
		if (!animstate->m_bInHitGroundAnimation)
		{
			if (just_landed || new_onladder)
			{
				int landing_activity = animstate->time_since_in_air() > 1.0f ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT;
				record->layers[5].m_nSequence = landing_activity;
				record->layers[5].m_flCycle = 0.0f;
				animstate->m_bInHitGroundAnimation = true;
			}
		}
	}

	if (!e->get_move_type() == MOVETYPE_LADDER)
	{
		if (record->layers[5].m_flWeight > 0.0f)
		{
			float v175 = (animstate->time_since_in_air() - 0.2f) * -5.0f;
			v175 = std::clamp(v175, 0.0f, 1.0f);
			float newlayer5_weight = ((3.0f - (v175 + v175)) * (v175 * v175)) * record->layers[5].m_flWeight;
			// set new weight.
			record->layers[5].m_flWeight = newlayer5_weight;
		}
	}


	if (previous_record)
	{
		memcpy(e->get_animlayers(), previous_record->layers, e->animlayer_count() * sizeof(AnimationLayer));

		auto ticks_chocked = 1;
		//auto simulation_ticks = TIME_TO_TICKS(e->m_flSimulationTime() - previous_record->simulation_time);

		//if (simulation_ticks > 0 && simulation_ticks < 17)
			//ticks_chocked = simulation_ticks;

		if (ticks_chocked > 1)
		{
			auto land_time = 0.0f;
			auto land_in_cycle = false;
			auto is_landed = false;
			auto on_ground = false;

			if (animlayers[4].m_flCycle < 0.5f && (!(e->m_fFlags() & FL_ONGROUND) || !(previous_record->flags & FL_ONGROUND)))
			{
				land_time = e->m_flSimulationTime() - animlayers[4].m_flPlaybackRate * animlayers[4].m_flCycle;
				land_in_cycle = land_time >= previous_record->simulation_time;
			}

			auto duck_amount_per_tick = (e->m_flDuckAmount() - previous_record->duck_amount) / ticks_chocked;

			for (auto i = 0; i < ticks_chocked; ++i)
			{
				auto simulated_time = previous_record->simulation_time + TICKS_TO_TIME(i);

				if (duck_amount_per_tick)
				{
					auto v208 = ((record->duck_amount - e->m_flDuckAmount()) * duck_amount_per_tick)
						+ e->m_flDuckAmount();

					e->m_flDuckAmount() = fminf(fmaxf(v208, 0.0f), 1.0f);

				}
				on_ground = e->m_fFlags() & FL_ONGROUND;

				if (land_in_cycle && !is_landed)
				{
					if (land_time <= simulated_time)
					{
						is_landed = true;
						on_ground = true;
					}
					else
						on_ground = previous_record->flags & FL_ONGROUND;
				}

				if (on_ground)
					e->m_fFlags() |= FL_ONGROUND;
				else
					e->m_fFlags() &= ~FL_ONGROUND;

				auto simulated_ticks = TIME_TO_TICKS(simulated_time);

				m_globals()->m_realtime = simulated_time;
				m_globals()->m_curtime = simulated_time;
				m_globals()->m_framecount = simulated_ticks;
				m_globals()->m_tickcount = simulated_ticks;
				m_globals()->m_interpolation_amount = 0.0f;

				//g_ctx.globals.updating_animation = true;
				e->update_clientside_animation();
				//g_ctx.globals.updating_animation = false;

				m_globals()->m_realtime = backup_realtime;
				m_globals()->m_curtime = backup_curtime;
				m_globals()->m_framecount = backup_framecount;
				m_globals()->m_tickcount = backup_tickcount;
				m_globals()->m_interpolation_amount = backup_interpolation_amount;

				updated_animations = true;
			}
		}
	}

	if (!updated_animations)
	{
		//g_ctx.globals.updating_animation = true;
		e->update_clientside_animation();
		//g_ctx.globals.updating_animation = false;
	}

	memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

	auto setup_matrix = [&](player_t* e, AnimationLayer* layers, const int& matrix) -> void
	{
		e->invalidate_physics_recursive(8);

		AnimationLayer backup_layers[15];
		memcpy(backup_layers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));

		memcpy(e->get_animlayers(), layers, e->animlayer_count() * sizeof(AnimationLayer));

		switch (matrix)
		{
		case MAIN:
			e->setup_bones_fixed(record->matrixes_data.main, BONE_USED_BY_ANYTHING);
			break;
		case NONE:
			e->setup_bones_fixed(record->matrixes_data.zero, BONE_USED_BY_HITBOX);
			break;
		case FIRST:
			e->setup_bones_fixed(record->matrixes_data.first, BONE_USED_BY_HITBOX);
			break;
		case SECOND:
			e->setup_bones_fixed(record->matrixes_data.second, BONE_USED_BY_HITBOX);
			break;
		}

		memcpy(e->get_animlayers(), backup_layers, e->animlayer_count() * sizeof(AnimationLayer));
	};

	if (!player_info.fakeplayer && g_ctx.local()->is_alive() && e->m_iTeamNum() != g_ctx.local()->m_iTeamNum())
	{
		animstate->m_flGoalFeetYaw = previous_goal_feet_yaw[e->EntIndex()]; //-V807

		g_ctx.globals.updating_animation = true;
		e->update_clientside_animation();
		g_ctx.globals.updating_animation = false;

		previous_goal_feet_yaw[e->EntIndex()] = animstate->m_flGoalFeetYaw;
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y); //-V807

		e->update_clientside_animation();

		setup_matrix(e, player_resolver[e->EntIndex()].resolver_layers[0], NONE);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + 60.0f);

		e->update_clientside_animation();

		setup_matrix(e, player_resolver[e->EntIndex()].resolver_layers[2], FIRST);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y - 60.0f);

		e->update_clientside_animation();

		setup_matrix(e, player_resolver[e->EntIndex()].resolver_layers[1], SECOND);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		player_resolver[e->EntIndex()].initialize(e, record, previous_goal_feet_yaw[e->EntIndex()], e->m_angEyeAngles().x, previous_record);
		//player_resolver[e->EntIndex()].resolve_yaw();
	}

	//g_ctx.globals.updating_animation = true;
	e->update_clientside_animation();
	//g_ctx.globals.updating_animation = false;

	setup_matrix(e, animlayers, MAIN);
	memcpy(e->m_CachedBoneData().Base(), record->matrixes_data.main, e->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

	m_globals()->m_curtime = backup_curtime;
	m_globals()->m_frametime = backup_frametime;

	e->m_flLowerBodyYawTarget() = backup_lower_body_yaw_target;
	e->m_flDuckAmount() = backup_duck_amount;
	e->m_fFlags() = backup_flags;
	e->m_iEFlags() = backup_eflags;

	memcpy(e->get_animlayers(), animlayers, e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(player_resolver[e->EntIndex()].previous_layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(player_resolver[e->EntIndex()].resolver_layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));

	record->store_data(e, false);

	if (e->m_flSimulationTime() < e->m_flOldSimulationTime())
		record->invalid = true;
}

void lagcompensation::FixPvs(player_t* pCurEntity)
{
	if (pCurEntity == g_ctx.local())
		return;

	if (!pCurEntity
		|| !pCurEntity->is_player()
		|| pCurEntity->EntIndex() == m_engine()->GetLocalPlayer())
		return;

	*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pCurEntity) + 0xA30) = m_globals()->m_framecount;
	*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pCurEntity) + 0xA28) = 0;
}