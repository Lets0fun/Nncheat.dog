// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "../ragebot/aim.h"
#include "fakelag.h"
#include "misc.h"
#include "prediction_system.h"
#include "logs.h"

void fakelag::Fakelag( CUserCmd* m_pcmd )
{
	if ( c_config::get( )->b[ "fakelag" ] && !g_ctx.globals.exploits )
	{
		static auto force_choke = false;

		if ( force_choke )
		{
			force_choke = false;
			g_ctx.send_packet = false;
			return;
		}

		if ( g_ctx.local( )->m_fFlags( ) & FL_ONGROUND && !( engineprediction::get( ).backup_data.flags & FL_ONGROUND ) )
		{
			force_choke = true;
			g_ctx.send_packet = false;
			return;
		}
	}

	auto choked = m_clientstate( )->iChokedCommands; //-V807
	auto flags = engineprediction::get( ).backup_data.flags; //-V807
	auto velocity = engineprediction::get( ).backup_data.velocity.Length( ); //-V807
	auto velocity2d = engineprediction::get( ).backup_data.velocity.Length2D( );

	auto max_speed = 260.0f;
	auto weapon_info = g_ctx.globals.weapon->get_csweapon_info( );

	if ( weapon_info )
		max_speed = g_ctx.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

	max_choke = c_config::get( )->i[ "fl_limit "];

	if ( m_gamerules( )->m_bIsValveDS( ) )
		max_choke = m_engine( )->IsVoiceRecording( ) ? 1 : min( max_choke, 6 );

	if ( g_ctx.local( )->m_fFlags( ) & FL_ONGROUND && engineprediction::get( ).backup_data.flags & FL_ONGROUND && !m_gamerules( )->m_bIsValveDS( ) && key_binds::get( ).get_key_bind_state( 20 ) )
	{
		max_choke = 14;

		if ( choked < max_choke )
			g_ctx.send_packet = false;
		else
			g_ctx.send_packet = true;
	}
	else
	{
		if ( !g_ctx.globals.exploits && c_config::get( )->b[ "fakelag" ] )
		{
			max_choke = c_config::get( )->i[ "fl_limit " ];

			if ( m_gamerules( )->m_bIsValveDS( ) )
				max_choke = min( max_choke, 6 );

			if ( choked < max_choke )
				g_ctx.send_packet = false;
			else
			{
				started_peeking = false;

				g_ctx.send_packet = true;
			}
		}
		else if ( g_ctx.globals.exploits || !antiaim::get( ).condition( m_pcmd, false ) && g_cfg.antiaim.desync )
		{
			condition = true;
			started_peeking = false;

			if ( choked < 1 )
				g_ctx.send_packet = false;
			else
				g_ctx.send_packet = true;
		}
		else
			condition = true;
	}
}

void fakelag::Createmove( )
{
	if ( FakelagCondition( g_ctx.get_command( ) ) )
		return;

	Fakelag( g_ctx.get_command( ) );

	if ( !m_gamerules( )->m_bIsValveDS( ) && m_clientstate( )->iChokedCommands <= 16 )
	{
		static auto Fn = util::FindSignature( "engine.dll", "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ) + 0x1;
		DWORD old = 0;

		VirtualProtect( ( void* )Fn, sizeof( uint32_t ), PAGE_EXECUTE_READWRITE, &old );
		*( uint32_t* )Fn = 17;
		VirtualProtect( ( void* )Fn, sizeof( uint32_t ), old, &old );
	}
}

bool fakelag::FakelagCondition( CUserCmd* m_pcmd )
{
	condition = false;

	if ( g_ctx.local( )->m_bGunGameImmunity( ) || g_ctx.local( )->m_fFlags( ) & FL_FROZEN )
		condition = true;

	if ( antiaim::get( ).freeze_check && !c_config::get()->auto_check( c_config::get( )->i[ "rage_dt_key" ], c_config::get( )->i[ "rage_dt_key_style" ] ) )
		condition = true;

	return condition;
}