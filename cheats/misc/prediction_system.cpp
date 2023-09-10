#include "prediction_system.h"

void engineprediction::FixNetvarCompression(int time)
{
    auto data = &netvars_data[time % MULTIPLAYER_BACKUP]; //-V807

    if (!data || !data->m_bIsFilled || data->command_number != time || (time - data->command_number) > 150)
        return;

    auto aim_punch_angle_delta = data->m_aimPunchAngle - g_ctx.local()->m_aimPunchAngle();
    auto aim_punch_angle_vel_delta = data->m_aimPunchAngleVel - g_ctx.local()->m_aimPunchAngleVel();
    auto view_punch_angle_delta = data->m_viewPunchAngle - g_ctx.local()->m_viewPunchAngle();
    auto view_offset_delta = data->m_vecViewOffset - g_ctx.local()->m_vecViewOffset();
    auto velocity_delta = data->m_vecVelocity - g_ctx.local()->m_vecVelocity();

    auto duck_amount_delta = data->m_flDuckAmount - g_ctx.local()->m_flDuckAmount();
    auto fall_velocity_delta = data->m_flFallVelocity - g_ctx.local()->m_flFallVelocity();
    auto velocity_modifier_delta = data->m_flVelocityModifier - g_ctx.local()->m_flVelocityModifier();

    if (std::fabs(aim_punch_angle_delta.x) <= 0.03125f && std::fabs(aim_punch_angle_delta.y) <= 0.03125f && std::fabs(aim_punch_angle_delta.z) <= 0.03125f)
        g_ctx.local()->m_aimPunchAngle() = data->m_aimPunchAngle;

    if (std::fabs(aim_punch_angle_vel_delta.x) <= 0.03125f && std::fabs(aim_punch_angle_vel_delta.y) <= 0.03125f && std::fabs(aim_punch_angle_vel_delta.z) <= 0.03125f)
        g_ctx.local()->m_aimPunchAngleVel() = data->m_aimPunchAngleVel;

    if (std::fabs(view_punch_angle_delta.x) <= 0.03125f && std::fabs(view_punch_angle_delta.y) <= 0.03125f && std::fabs(view_punch_angle_delta.z) <= 0.03125f)
        g_ctx.local()->m_viewPunchAngle() = data->m_viewPunchAngle;

    if (std::fabs(view_offset_delta.x) <= 0.03125f && std::fabs(view_offset_delta.y) <= 0.03125f && std::fabs(view_offset_delta.z) <= 0.03125f)
        g_ctx.local()->m_vecViewOffset() = data->m_vecViewOffset;

    if (std::fabs(velocity_delta.x) <= 0.03125f && std::fabs(velocity_delta.y) <= 0.03125f && std::fabs(velocity_delta.z) <= 0.03125f)
        g_ctx.local()->m_vecVelocity() = data->m_vecVelocity;



    if (std::fabs(duck_amount_delta) <= 0.03125f)
        g_ctx.local()->m_flDuckAmount() = data->m_flDuckAmount;

    if (std::fabs(fall_velocity_delta) <= 0.03125f)
        g_ctx.local()->m_flFallVelocity() = data->m_flFallVelocity;

    if (std::fabs(velocity_modifier_delta) <= 0.00625f)
        g_ctx.local()->m_flVelocityModifier() = data->m_flVelocityModifier;
}

void engineprediction::store_netvars()
{
    auto data = &netvars_data[m_clientstate()->iCommandAck % MULTIPLAYER_BACKUP];

    data->tickbase = g_ctx.local()->m_nTickBase();
    data->m_aimPunchAngle = g_ctx.local()->m_aimPunchAngle();
    data->m_aimPunchAngleVel = g_ctx.local()->m_aimPunchAngleVel();
    data->m_viewPunchAngle = g_ctx.local()->m_viewPunchAngle();
    data->m_vecViewOffset = g_ctx.local()->m_vecViewOffset();
    data->m_duckAmount = g_ctx.local()->m_flDuckAmount();
    data->m_duckSpeed = g_ctx.local()->m_flDuckSpeed();
}

void engineprediction::store_data()
{
    int          tickbase;
    StoredData_t* data;

    if (!g_ctx.local() && !g_ctx.local()->is_alive()) {
        reset_data();
        return;
    }

    tickbase = g_ctx.local()->m_nTickBase();

    data = &m_data[tickbase % MULTIPLAYER_BACKUP];

    data->m_tickbase = tickbase;
    data->m_punch = g_ctx.local()->m_aimPunchAngle();
    data->m_punch_vel = g_ctx.local()->m_aimPunchAngleVel();
    data->m_view_offset = g_ctx.local()->m_vecViewOffset();
    data->m_velocity_modifier = g_ctx.local()->m_flVelocityModifier();
}

void engineprediction::reset_data()
{
    m_data.fill(StoredData_t());
}

void engineprediction::update_incoming_sequences()
{
    if (!m_clientstate()->pNetChannel)
        return;

    if (m_sequence.empty() || m_clientstate()->pNetChannel->m_nInSequenceNr > m_sequence.front().m_seq) {
        m_sequence.emplace_front(m_globals()->m_realtime, m_clientstate()->pNetChannel->m_nInReliableState, m_clientstate()->pNetChannel->m_nInSequenceNr);
    }

    while (m_sequence.size() > 2048)
        m_sequence.pop_back();
}

void engineprediction::update_vel()
{
    static int m_iLastCmdAck = 0;
    static float m_flNextCmdTime = 0.f;

    if (m_clientstate() && (m_iLastCmdAck != m_clientstate()->nLastCommandAck || m_flNextCmdTime != m_clientstate()->flNextCmdTime))
    {
        if (g_ctx.globals.last_velocity_modifier != g_ctx.local()->m_flVelocityModifier())
        {
            *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(m_prediction() + 0x24)) = 1;
            g_ctx.globals.last_velocity_modifier = g_ctx.local()->m_flVelocityModifier();
        }

        m_iLastCmdAck = m_clientstate()->nLastCommandAck;
        m_flNextCmdTime = m_clientstate()->flNextCmdTime;
    }
}

void engineprediction::setup()
{
    if (prediction_data.prediction_stage != SETUP)
        return;

    backup_data.flags = g_ctx.local()->m_fFlags(); //-V807
    backup_data.velocity = g_ctx.local()->m_vecVelocity();

    prediction_data.old_curtime = m_globals()->m_curtime; //-V807
    prediction_data.old_frametime = m_globals()->m_frametime;

    m_prediction()->InPrediction = true;
    m_prediction()->IsFirstTimePredicted = false;

    m_globals()->m_curtime = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);
    m_globals()->m_frametime = m_prediction()->EnginePaused ? 0.0f : m_globals()->m_intervalpertick;

    prediction_data.prediction_stage = PREDICT;
}

void engineprediction::predict(CUserCmd* m_pcmd) {
    static auto oldorigin = g_ctx.local()->m_vecOrigin();
    auto unpred_vel = (g_ctx.local()->m_vecOrigin() - oldorigin) * (1.0 / m_globals()->m_intervalpertick);
    oldorigin = g_ctx.local()->m_vecOrigin();

   // auto unpred_eyepos = g_ctx.local()->G();


    if (!prediction_data.prediction_random_seed)
        prediction_data.prediction_random_seed = *reinterpret_cast <int**> (util::FindSignature(crypt_str("client.dll"), crypt_str("A3 ? ? ? ? 66 0F 6E 86")) + 0x1);

    *prediction_data.prediction_random_seed = MD5_PseudoRandom(m_pcmd->m_command_number) & INT_MAX;

    if (!prediction_data.prediction_player)
        prediction_data.prediction_player = *reinterpret_cast <int**> (util::FindSignature(crypt_str("client.dll"), crypt_str("89 35 ? ? ? ? F3 0F 10 48")) + 0x2);

    *prediction_data.prediction_player = reinterpret_cast <int> (g_ctx.local());

    prediction_data.old_curtime = m_globals()->m_curtime; //-V807
    prediction_data.old_frametime = m_globals()->m_frametime;

    m_globals()->m_curtime = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);
    m_globals()->m_frametime = m_prediction()->EnginePaused ? 0.0f : m_globals()->m_intervalpertick;

    m_gamemovement()->StartTrackPredictionErrors(g_ctx.local());

    CMoveData move_data;
    memset(&move_data, 0, sizeof(CMoveData));
    m_gamemovement()->StartTrackPredictionErrors(g_ctx.local());
    m_movehelper()->set_host(g_ctx.local());
    m_prediction()->SetupMove(g_ctx.local(), m_pcmd, m_movehelper(), &move_data);
    m_prediction()->FinishMove(g_ctx.local(), m_pcmd, &move_data);

    static auto pred_oldorigin = g_ctx.local()->m_vecOrigin();
    auto pred_vel = (g_ctx.local()->m_vecOrigin() - pred_oldorigin) * (1.0 / m_globals()->m_intervalpertick);
    pred_oldorigin = g_ctx.local()->m_vecOrigin();

    //unpred_eyepos = g_ctx.local()->m_vecOrigin();

    prediction_data.prediction_stage = FINISH;
}

void engineprediction::OnFrameStageNotify(ClientFrameStage_t Stage)
{
    // local must be alive and we also must receive an update
    if (Stage != ClientFrameStage_t::FRAME_NET_UPDATE_END || !g_ctx.local()->is_alive())
        return;

    // define const
    const int nSimulationTick = TIME_TO_TICKS(g_ctx.local()->m_flSimulationTime());
    const int nOldSimulationTick = TIME_TO_TICKS(g_ctx.local()->m_flOldSimulationTime());
    const int nCorrectionTicks = TIME_TO_TICKS(0.03f);
    const int nServerTick = m_clientstate()->m_iServerTick;

    // check time
    if (nSimulationTick <= nOldSimulationTick || abs(nSimulationTick - nServerTick) > nCorrectionTicks)
        return;

    // save last simulation ticks amount
   // m_TickBase.m_nSimulationTicks = nSimulationTick - nServerTick;
}

void engineprediction::finish()
{
    if (prediction_data.prediction_stage != FINISH)
        return;

    m_gamemovement()->StartTrackPredictionErrors(g_ctx.local());
    m_movehelper()->set_host(g_ctx.local());

    *prediction_data.prediction_random_seed = -1;
    *prediction_data.prediction_player = 0;

    m_globals()->m_curtime = prediction_data.old_curtime;
    m_globals()->m_frametime = prediction_data.old_frametime;
}