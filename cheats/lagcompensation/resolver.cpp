#include "animation_system.h"
#include "..\ragebot\aim.h"
#include "../visuals/player_esp.h"

void resolver::initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch, adjust_data* previous_record)
{
    player = e;
    player_record = record;

    if (previous_record)
        prev_record = previous_record;

    original_pitch = math::normalize_pitch(pitch);
    original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
}

void resolver::lagcomp_initialize(player_t* player, Vector& origin, Vector& velocity, int& flags, bool on_ground)
{
    lagcompensation::get().extrapolate(player, origin, velocity, flags, on_ground);
}

void resolver::reset()
{
    player = nullptr;
    player_record = nullptr;
    prev_record = nullptr;

    side = false;
    fake = false;

    was_first_bruteforce = false;
    was_second_bruteforce = false;

    original_goal_feet_yaw = 0.0f;
    original_pitch = 0.0f;
}

bool resolver::is_breaking_lby(AnimationLayer cur_layer, AnimationLayer prev_layer)
{
    if (IsAdjustingBalance())
    {
        if ((prev_layer.m_flCycle != cur_layer.m_flCycle) && cur_layer.m_flWeight == 1.0f)
        {
            return true;
        }
        else if (cur_layer.m_flWeight == 0.0f && (prev_layer.m_flCycle > 0.92f && cur_layer.m_flCycle > 0.92f))
        {
            return true;
        }
    }

    return false;
}

bool resolver::IsAdjustingBalance()
{
    for (int i = 0; i < 13; i++)
    {
        const int activity = player->sequence_activity(player_record->layers[i].m_nSequence);
        if (activity == 979)
        {
            return true;
        }
    }

    return false;
}

bool resolver::is_slow_walking()
{
    static float old_velocity_2D[64] = { 0.0f };
    static int tick_counter[64] = { 0 };

    Vector velocity = player->m_vecVelocity();
    float velocity_2D = velocity.Length2D();

    if (velocity_2D != old_velocity_2D[player->EntIndex()])
    {
        old_velocity_2D[player->EntIndex()] = velocity_2D;
        tick_counter[player->EntIndex()] = 0;
    }
    else
    {
        tick_counter[player->EntIndex()]++;

        int max_ticks = static_cast<int>(0.1f / m_globals()->m_intervalpertick);

        if (tick_counter[player->EntIndex()] > max_ticks)
            return true;
    }

    return false;
}

int last_ticks[65] = { 0 };
int resolver::GetChokedPackets()
{
    int ticks = TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime());

    if (ticks == 0 && last_ticks[player->EntIndex()] > 0)
    {
        return last_ticks[player->EntIndex()] - 1;
    }
    else
    {
        last_ticks[player->EntIndex()] = ticks;
        return ticks;
    }
}

float angle_diff_onetap(float a1, float a2)
{
    while (a1 > 180.0f)
        a1 -= 360.0f;
    while (a1 < -180.0f)
        a1 += 360.0f;

    while (a2 > 180.0f)
        a2 -= 360.0f;
    while (a2 < -180.0f)
        a2 += 360.0f;

    float angle_diff = a2 - a1;

    while (angle_diff > 180.0f)
        angle_diff -= 360.0f;
    while (angle_diff < -180.0f)
        angle_diff += 360.0f;

    return angle_diff;
}

void resolver::get_side_standing()
{
    float angle_difference = math::AngleDiff(player->m_angEyeAngles().y, original_goal_feet_yaw);
    player_record->curSide = (angle_difference <= 0.0f) ? LEFT1 : RIGHT1;
}

float_t get_backward_side(player_t* player)
{
    return math::calculate_angle(g_ctx.local()->m_vecOrigin(), player->m_vecOrigin()).y;
}

static auto resolve_update_animations(player_t* e)
{
    e->update_clientside_animation();
}

Vector GetHitboxPos(player_t* player, matrix3x4_t* mat, int hitbox_id)
{
    if (!player)
        return Vector();

    auto hdr = m_modelinfo()->GetStudioModel(player->GetModel());

    if (!hdr)
        return Vector();

    auto hitbox_set = hdr->pHitboxSet(player->m_nHitboxSet());

    if (!hitbox_set)
        return Vector();

    auto hitbox = hitbox_set->pHitbox(hitbox_id);

    if (!hitbox)
        return Vector();

    Vector min, max;
    math::vector_transform(hitbox->bbmin, mat[hitbox->bone], min);
    math::vector_transform(hitbox->bbmax, mat[hitbox->bone], max);

    return (min + max) * 0.5f;
}

void resolve_yaw()
{

}