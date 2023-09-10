#include "local_animations.h"

void local_animations::run(ClientFrameStage_t stage)
{
    const float WEIGHT_CYCLE_RESET = 0.0f;
    const int LAYER_FAKE_WEIGHT = 3;
    const int LAYER_FAKE_CYCLE = 3;
    const int LAYER_YAW_WEIGHT = 12;

    if (stage == FRAME_NET_UPDATE_END)
    {
        if (!fakelag::get().condition && key_binds::get().get_key_bind_state(20))
        {
            fake_server_update = false;

            if (g_ctx.local()->m_flSimulationTime() != fake_simulation_time)
            {
                fake_server_update = true;
                fake_simulation_time = g_ctx.local()->m_flSimulationTime();
            }

            reset_anim_layer(LAYER_FAKE_WEIGHT, WEIGHT_CYCLE_RESET);
            reset_anim_layer(LAYER_FAKE_CYCLE, WEIGHT_CYCLE_RESET);
            reset_anim_layer(LAYER_YAW_WEIGHT, WEIGHT_CYCLE_RESET);

            update_fake_animations();
        }
    }
    else if (stage == FRAME_RENDER_START)
    {
        auto animstate = g_ctx.local()->get_animation_state();

        if (!animstate)
            return;

        real_server_update = false;

        if (g_ctx.local()->m_flSimulationTime() != real_simulation_time)
        {
            real_server_update = true;
            real_simulation_time = g_ctx.local()->m_flSimulationTime();
        }

        if (c_config::get()->b["legsinair"])
        {
            animstate->time_since_in_air() = 99.f;
            g_ctx.local()->get_animlayers()[5].m_flCycle = 0.f;
        }

        update_local_animations(*animstate);
    }
    else if (stage == FRAME_RENDER_START) // This block is duplicated, you may want to remove it.
    {
        auto animstate = *g_ctx.local()->get_animation_state();

        if (!g_ctx.local()->get_animation_state())
            return;

        real_server_update = false;
        fake_server_update = false;

        if (g_ctx.local()->m_flSimulationTime() != real_simulation_time || g_ctx.local()->m_flSimulationTime() != fake_simulation_time)
        {
            real_server_update = fake_server_update = true;
            real_simulation_time = fake_simulation_time = g_ctx.local()->m_flSimulationTime();
        }

        reset_anim_layer(LAYER_FAKE_WEIGHT, WEIGHT_CYCLE_RESET);
        reset_anim_layer(LAYER_FAKE_CYCLE, WEIGHT_CYCLE_RESET);
        reset_anim_layer(LAYER_YAW_WEIGHT, WEIGHT_CYCLE_RESET);

        update_local_animations(animstate);

        if (c_config::get()->b["legsinair"])
        {
            animstate.time_since_in_air() = 99.f;
            g_ctx.local()->get_animlayers()[5].m_flCycle = 0.f;
        }

        if (fakelag::get().condition && key_binds::get().get_key_bind_state(20))
        {
            fake_server_update = false;

            if (g_ctx.local()->m_flSimulationTime() != fake_simulation_time)
            {
                fake_server_update = true;
                fake_simulation_time = g_ctx.local()->m_flSimulationTime();
            }

            update_fake_animations();
        }
    }
}


void local_animations::reset_anim_layer( int layer, float weight )
{
    auto& anim_layer = g_ctx.local( )->get_animlayers( )[ layer ];
    anim_layer.m_flWeight = weight;
    anim_layer.m_flCycle = weight;
}

void local_animations::update_prediction_animations( )
{
    bool alloc = !local_data.prediction_animstate;
    bool change = !alloc && handle != &g_ctx.local( )->GetRefEHandle( );
    bool reset = !alloc && !change && g_ctx.local( )->m_flSpawnTime( ) != spawntime;
    if ( change ) {
        m_memalloc( )->Free( local_data.prediction_animstate );
    }

    if ( reset ) {
        util::reset_state( local_data.prediction_animstate );
        spawntime = g_ctx.local( )->m_flSpawnTime( );
    }

    if ( alloc || change ) {
        local_data.prediction_animstate = ( c_baseplayeranimationstate* )m_memalloc( )->Alloc( sizeof( c_baseplayeranimationstate ) );
        if ( local_data.prediction_animstate ) {
            util::create_state( local_data.prediction_animstate, g_ctx.local( ) );
        }
        handle = ( CBaseHandle* )&g_ctx.local( )->GetRefEHandle( );
        spawntime = g_ctx.local( )->m_flSpawnTime( );
    }

    if ( !alloc && !change && !reset ) {
        float pose_parameter[ 24 ];
        std::memcpy( pose_parameter, &g_ctx.local( )->m_flPoseParameter( ), 24 * sizeof( float ) );

        AnimationLayer layers[ 13 ];
        std::memcpy( layers, g_ctx.local( )->get_animlayers( ), g_ctx.local( )->animlayer_count( ) * sizeof( AnimationLayer ) );

        local_data.prediction_animstate->m_pBaseEntity = g_ctx.local( );
        util::update_state( local_data.prediction_animstate, ZERO );

        g_ctx.local( )->setup_bones_fixed( g_ctx.globals.prediction_matrix, BONE_USED_BY_HITBOX );

        std::memcpy( &g_ctx.local( )->m_flPoseParameter( ), pose_parameter, 24 * sizeof( float ) );
        std::memcpy( g_ctx.local( )->get_animlayers( ), layers, g_ctx.local( )->animlayer_count( ) * sizeof( AnimationLayer ) );
    }
}

void local_animations::update_fake_animations()
{
    auto alloc = !local_data.animstate;
    auto change = !alloc && handle != &g_ctx.local()->GetRefEHandle();
    auto reset = !alloc && !change && g_ctx.local()->m_flSpawnTime() != spawntime;

    if (change)
        m_memalloc()->Free(local_data.animstate);

    if (reset)
    {
        util::reset_state(local_data.animstate);
        spawntime = g_ctx.local()->m_flSpawnTime();
    }

    if (alloc || change)
    {
        local_data.animstate = (c_baseplayeranimationstate*)m_memalloc()->Alloc(sizeof(c_baseplayeranimationstate));

        if (local_data.animstate)
            util::create_state(local_data.animstate, g_ctx.local());

        handle = (CBaseHandle*)&g_ctx.local()->GetRefEHandle();
        spawntime = g_ctx.local()->m_flSpawnTime();
    }

    if (!alloc && !change && !reset && fake_server_update)
    {
        float pose_parameter[24];
        memcpy(pose_parameter, &g_ctx.local()->m_flPoseParameter(), 24 * sizeof(float));

        AnimationLayer layers[15];
        memcpy(layers, g_ctx.local()->get_animlayers(), g_ctx.local()->animlayer_count() * sizeof(AnimationLayer));

        auto backup_frametime = m_globals()->m_frametime;
        auto backup_curtime = m_globals()->m_curtime;

        m_globals()->m_frametime = m_globals()->m_intervalpertick;
        m_globals()->m_curtime = g_ctx.local()->m_flSimulationTime();

        local_data.animstate->m_pBaseEntity = g_ctx.local();
        util::update_state(local_data.animstate, local_animations::get().local_data.fake_angles);

        local_data.animstate->m_bInHitGroundAnimation = false;
        local_data.animstate->m_fLandingDuckAdditiveSomething = 0.0f;
        local_data.animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation = 1.0f;

        g_ctx.local()->setup_bones_fixed(g_ctx.globals.fake_matrix, BONE_USED_BY_ANYTHING);
        //local_data.visualize_lag = g_cfg.player.visualize_lag;

        //if (!local_data.visualize_lag)
        //{
        //    for (auto& i : g_ctx.globals.fake_matrix)
        //    {
        //        i[0][3] -= g_ctx.local()->GetRenderOrigin().x;
        //        i[1][3] -= g_ctx.local()->GetRenderOrigin().y;
        //        i[2][3] -= g_ctx.local()->GetRenderOrigin().z;
        //    }
        //}

        m_globals()->m_frametime = backup_frametime;
        m_globals()->m_curtime = backup_curtime;

        memcpy(&g_ctx.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
        memcpy(g_ctx.local()->get_animlayers(), layers, g_ctx.local()->animlayer_count() * sizeof(AnimationLayer));
    }
}

void local_animations::update_local_animations(c_baseplayeranimationstate& animstate)
{
    if (tickcount != m_globals()->m_tickcount)
    {
        tickcount = m_globals()->m_tickcount;

        update_animlayers();

        update_animstate(animstate);

        update_abs_angles();

        save_pose_parameter();
    }
    else
    {
        animstate.m_iLastClientSideAnimationUpdateFramecount = m_globals()->m_framecount;
    }

    update_goal_feet_yaw(animstate);

    update_abs_angles_in_entity(animstate);

    restore_pose_parameter();
}


void local_animations::update_animlayers( )
{
    memcpy( layers, g_ctx.local( )->get_animlayers( ), g_ctx.local( )->animlayer_count( ) * sizeof( AnimationLayer ) );
}

void local_animations::update_animstate( c_baseplayeranimationstate& animstate )
{
    if ( local_data.animstate )
    {
        animstate.m_fDuckAmount = local_data.animstate->m_fDuckAmount;
    }

    animstate.m_iLastClientSideAnimationUpdateFramecount = 0;
    util::update_state( &animstate, local_animations::get( ).local_data.fake_angles );
}

void local_animations::update_abs_angles( )
{
    if ( real_server_update )
    {
        abs_angles = local_animations::get( ).local_data.real_angles.y;
    }
    else
    {
        abs_angles = antiaim::get( ).condition( g_ctx.get_command( ) ) ? abs_angles : local_animations::get( ).local_data.real_angles.y;
    }
}

void local_animations::save_pose_parameter( )
{
    memcpy( pose_parameter, &g_ctx.local( )->m_flPoseParameter( ), 24 * sizeof( float ) );
}

void local_animations::update_goal_feet_yaw( c_baseplayeranimationstate& animstate )
{
    animstate.m_flGoalFeetYaw = abs_angles;
}

void local_animations::update_abs_angles_in_entity( c_baseplayeranimationstate& animstate )
{
    g_ctx.local( )->set_abs_angles( Vector( 0.0f, abs_angles, 0.0f ) );
    memcpy( g_ctx.local( )->get_animlayers( ), layers, g_ctx.local( )->animlayer_count( ) * sizeof( AnimationLayer ) );
}

void local_animations::restore_pose_parameter( )
{
    memcpy( &g_ctx.local( )->m_flPoseParameter( ), pose_parameter, 24 * sizeof( float ) );
}