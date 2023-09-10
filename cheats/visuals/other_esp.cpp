#include "other_esp.h"
#include "..\autowall\autowall.h"
#include "..\ragebot\antiaim.h"
#include "..\misc\logs.h"
#include "..\misc\misc.h"
#include "../Configuration/Config.h"
#include "..\lagcompensation\local_animations.h"
#include "../Menu/MenuFramework/Framework.h"
#include "../Menu/MenuFramework/Renderer.h"
#include "..\..\utils\Render.h"
#include "../menu_alpha.h"

using namespace IdaLovesMe;

bool can_penetrate( weapon_t* weapon )
{
	auto weapon_info = weapon->get_csweapon_info( );

	if ( !weapon_info )
		return false;

	Vector view_angles;
	m_engine( )->GetViewAngles( view_angles );

	Vector direction;
	math::angle_vectors( view_angles, direction );

	CTraceFilter filter;
	filter.pSkip = g_ctx.local( );

	trace_t trace;
	util::trace_line( g_ctx.globals.eye_pos, g_ctx.globals.eye_pos + direction * weapon_info->flRange, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace );

	if ( trace.fraction == 1.0f ) //-V550
		return false;

	auto eye_pos = g_ctx.globals.eye_pos;
	auto hits = 1;
	auto damage = ( float )weapon_info->iDamage;
	auto penetration_power = weapon_info->flPenetration;

	static auto damageReductionBullets = m_cvar( )->FindVar( crypt_str( "ff_damage_reduction_bullets" ) );
	static auto damageBulletPenetration = m_cvar( )->FindVar( crypt_str( "ff_damage_bullet_penetration" ) );

	return autowall::get( ).handle_bullet_penetration( weapon_info, trace, eye_pos, direction, hits, damage, penetration_power, damageReductionBullets->GetFloat( ), damageBulletPenetration->GetFloat( ) );
}

void otheresp::penetration_reticle( )
{
	if ( !c_config::get( )->auto_check( c_config::get( )->i[ "esp_en" ], c_config::get( )->i[ "esp_en_type" ] ) )
		return;

	if ( !c_config::get( )->b[ "penrect" ] )
		return;

	if ( !g_ctx.local( )->is_alive( ) )
		return;

	auto weapon = g_ctx.local( )->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return;

	auto color = Color::Red;

	if ( !weapon->is_non_aim( ) && weapon->m_iItemDefinitionIndex( ) != WEAPON_TASER && can_penetrate( weapon ) )
		color = Color::Green;

	static int width, height;
	m_engine( )->GetScreenSize( width, height );

	render::get( ).rect_filled( width / 2, height / 2 - 1, 1, 3, color );
	render::get( ).rect_filled( width / 2 - 1, height / 2, 3, 1, color );
}

enum key_bind_num
{
	_AUTOFIRE,
	_LEGITBOT,
	_DOUBLETAP,
	_SAFEPOINT,
	_MIN_DAMAGE,
	_ANTI_BACKSHOT = 12,
	_M_BACK,
	_M_LEFT,
	_M_RIGHT,
	_DESYNC_FLIP,
	_THIRDPERSON,
	_AUTO_PEEK,
	_EDGE_JUMP,
	_FAKEDUCK,
	_SLOWWALK,
	_BODY_AIM,
	_RAGEBOT,
	_TRIGGERBOT,
	_L_RESOLVER_OVERRIDE,
	_FAKE_PEEK,
};



void otheresp::indicators( )
{
	static int height, width;
	m_engine( )->GetScreenSize( width, height );

	g_ctx.globals.indicator_pos = height / 2;

	static auto percent_col = [ ] ( int per ) -> Color {
		int red = per < 50 ? 255 : floorf( 255 - ( per * 2 - 100 ) * 255.f / 100.f );
		int green = per > 50 ? 255 : floorf( ( per * 2 ) * 255.f / 100.f );

		return Color( red, green, 0 );
	};
	auto cfg = c_config::get( );

	auto weapon = g_ctx.local( )->m_hActiveWeapon( ).Get( );

	if ( cfg->m[ "ftind" ][ 0 ] && cfg->auto_check( cfg->i[ "rage_sp_enabled" ], cfg->i[ "rage_sp_enabled_style" ] ) )
	{
		otheresp::get( ).m_indicators.push_back( { "SP", D3DCOLOR_RGBA( 255, 255, 0,255 ) } );
	}

	if ( cfg->m[ "ftind" ][ 1 ] && cfg->auto_check( cfg->i[ "rage_baim_enabled" ], cfg->i[ "rage_baim_enabled_style" ] ) )
	{
		otheresp::get( ).m_indicators.push_back( { "BAIM", D3DCOLOR_RGBA( 255, 255, 255,255 ) } );
	}

	Color PINGCOL = percent_col( ( float( std::clamp( g_ctx.globals.ping, 30, 200 ) - 64 ) ) / 200.f * 100.f );
	if ( cfg->m[ "ftind" ][ 2 ] && ( cfg->b[ "misc_ping_spike" ] && cfg->auto_check( cfg->i[ "ping_spike_key" ], cfg->i[ "ping_spike_key_style" ] ) ) )
	{
		otheresp::get( ).m_indicators.push_back( { "PING", D3DCOLOR_RGBA( PINGCOL.r( ), PINGCOL.g( ), PINGCOL.b( ),255 ) } );
	}

	if ( cfg->m[ "ftind" ][ 3 ] && ( cfg->b[ "rage_dt" ] && cfg->auto_check( cfg->i[ "rage_dt_key" ], cfg->i[ "rage_dt_key_style" ] ) ) )
	{
		otheresp::get( ).m_indicators.push_back( { "DT",  weapon->can_double_tap_for_ind( ) && weapon->can_fire( false ) ? D3DCOLOR_RGBA( 255, 255, 255,255 ) : D3DCOLOR_RGBA( 255, 0, 0, 255 ) } );
	}

	if ( cfg->m[ "ftind" ][ 4 ] && cfg->auto_check( cfg->i[ "rage_fd_enabled" ], cfg->i[ "rage_fd_enabled_style" ] ) )
	{
		otheresp::get( ).m_indicators.push_back( { "DUCK", D3DCOLOR_RGBA( 255, 255, 255,255 ) } );
	}

	if ( cfg->m[ "ftind" ][ 5 ] && cfg->i[ "aa_fs" ] == 0 && cfg->auto_check( cfg->i[ "freestand_key" ], cfg->i[ "freestand_key_style" ] ) )
	{
		otheresp::get( ).m_indicators.push_back( { "FS", D3DCOLOR_RGBA( 255, 255, 255,255 ) } );
	}

	Color FPSCOL = percent_col( ( float( std::clamp( g_ctx.globals.framerate, 30, 80 ) - 64 ) ) / 200.f * 100.f );

	if ( cfg->b[ "fps_warning" ] )
	{
		otheresp::get( ).m_indicators.push_back( { "FPS", D3DCOLOR_RGBA( FPSCOL.r( ), FPSCOL.g( ), FPSCOL.b( ),255 ) } );
	}
	if ( cfg->m[ "ftind" ][ 6 ] && ( cfg->b[ "hideshots" ] && cfg->auto_check( cfg->i[ "hs_key" ], cfg->i[ "hideshots_key_style" ] ) ) )
	{
		otheresp::get( ).m_indicators.push_back( { "OSAA", D3DCOLOR_RGBA( 157, 153, 156, 255 ) } );
	}
	if ( cfg->m[ "ftind" ][ 7 ] && ( cfg->b[ "override_dmg_bool" ] && cfg->auto_check( cfg->i[ "min_dmg_override_key" ], cfg->i[ "min_dmg_override_key_style" ] ) ) )
	{
		otheresp::get( ).m_indicators.push_back( { "DMG", D3DCOLOR_RGBA( 255, 255, 255,255 ) } );
	}

}

void otheresp::create_fonts( ) {

	static auto create_font2 = [ ] ( const char* name, int size, int weight, DWORD flags ) -> vgui::HFont
	{
		g_ctx.last_font_name = name;

		auto font = m_surface( )->FontCreate( );
		m_surface( )->SetFontGlyphSet( font, name, size, weight, 0, 0, flags );

		return font;
	};

	IndFont = create_font2( "Calibri", 30, 600, FONTFLAG_ANTIALIAS );
	IndShadow = create_font2( "Calibri", 30, 600, FONTFLAG_ANTIALIAS );
}

void otheresp::draw_indicators( )
{
	//if (!g_ctx.local()->is_alive()) //-V807
		//return;

	static int width, height;
	m_engine( )->GetScreenSize( width, height );

	int h = 0;

	for ( auto& indicator : m_indicators )
	{

		render::get( ).gradient( 17, height - 300 - h - 3, render::get( ).text_width( IndFont, indicator.str.c_str( ) ) / 2, render::get( ).text_height( IndFont, indicator.str.c_str( ) ) + 8, Color( 0, 0, 0, 0 ), Color( 0, 0, 0, 165 ), GRADIENT_HORIZONTAL );
		render::get( ).gradient( 17 + render::get( ).text_width( IndFont, indicator.str.c_str( ) ) / 2, height - 300 - h - 3, render::get( ).text_height( IndFont, indicator.str.c_str( ) ) + 8, render::get( ).text_height( IndFont, indicator.str.c_str( ) ) + 8, Color( 0, 0, 0, 165 ), Color( 0, 0, 0, 0 ), GRADIENT_HORIZONTAL );
		render::get( ).text( IndFont, 27, height - 300 - h, Color( 0, 0, 0, 200 ), HFONT_CENTERED_NONE, indicator.str.c_str( ) );
		render::get( ).text( IndFont, 27 - 1, height - 300 - h - 1, Color( ( int )get_r( indicator.color ), ( int )get_g( indicator.color ), ( int )get_b( indicator.color ), ( int )get_a( indicator.color ) ), HFONT_CENTERED_NONE, indicator.str.c_str( ) );
		h += 40;
	}

	m_indicators.clear( );
}

void otheresp::hitmarker_paint( )
{
	auto cfg = c_config::get( );
	if ( !cfg->b[ "hitmarker" ] )
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if ( !g_ctx.local( )->is_alive( ) )
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if ( !cfg->b[ "hitmarker" ] )
		return;

	if ( hitmarker.hurt_time + 0.7f > m_globals( )->m_curtime )
	{

		static int width, height;
		m_engine( )->GetScreenSize( width, height );

		auto alpha = ( int )( ( hitmarker.hurt_time + 0.7f - m_globals( )->m_curtime ) * 255.0f );
		hitmarker.hurt_color.SetAlpha( alpha );

		auto offset = 7.0f - ( float )alpha / 255.0f * 7.0f;

		render::get( ).line( width / 2 + 5, height / 2 - 5, width / 2 + 10, height / 2 - 10, Color::White );
		render::get( ).line( width / 2 + 5, height / 2 + 5, width / 2 + 10, height / 2 + 10, Color::White );
		render::get( ).line( width / 2 - 5, height / 2 + 5, width / 2 - 10, height / 2 + 10, Color::White );
		render::get( ).line( width / 2 - 5, height / 2 - 5, width / 2 - 10, height / 2 - 10, Color::White );
	}
}


void draw_circe( float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device );

void otheresp::spread_crosshair( LPDIRECT3DDEVICE9 device )
{
	//
}

void draw_circe( float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device )
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2 = nullptr;
	std::vector <CUSTOMVERTEX2> circle( resolution + 2 );

	circle[ 0 ].x = x;
	circle[ 0 ].y = y;
	circle[ 0 ].z = 0.0f;

	circle[ 0 ].rhw = 1.0f;
	circle[ 0 ].color = color2;

	for ( auto i = 1; i < resolution + 2; i++ )
	{
		circle[ i ].x = ( float )( x - radius * cos( D3DX_PI * ( ( i - 1 ) / ( resolution / 2.0f ) ) ) );
		circle[ i ].y = ( float )( y - radius * sin( D3DX_PI * ( ( i - 1 ) / ( resolution / 2.0f ) ) ) );
		circle[ i ].z = 0.0f;

		circle[ i ].rhw = 1.0f;
		circle[ i ].color = color;
	}

	device->CreateVertexBuffer( ( resolution + 2 ) * sizeof( CUSTOMVERTEX2 ), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, nullptr ); //-V107

	if ( !g_pVB2 )
		return;

	void* pVertices;

	g_pVB2->Lock( 0, ( resolution + 2 ) * sizeof( CUSTOMVERTEX2 ), ( void** )&pVertices, 0 ); //-V107
	memcpy( pVertices, &circle[ 0 ], ( resolution + 2 ) * sizeof( CUSTOMVERTEX2 ) );
	g_pVB2->Unlock( );

	device->SetTexture( 0, nullptr );
	device->SetPixelShader( nullptr );
	device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	device->SetStreamSource( 0, g_pVB2, 0, sizeof( CUSTOMVERTEX2 ) );
	device->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	device->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, resolution );

	g_pVB2->Release( );
}

void otheresp::automatic_peek_indicator( )
{
	auto weapon = g_ctx.local( )->m_hActiveWeapon( ).Get( );

	auto color_main = Color( c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 0 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 1 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 2 ], 25 );

	if ( !weapon )
		return;

	static auto position = ZERO;

	if ( !g_ctx.globals.start_position.IsZero( ) )
		position = g_ctx.globals.start_position;

	if ( position.IsZero( ) )
		return;

	static auto alpha = 0.0f;

	Vector screen;
	Vector quic;

	Vector local_origin = g_ctx.local( )->GetAbsOrigin( );
	Vector localorign;

	if ( c_config::get( )->b[ "rage_quick_peek_assist" ] && c_config::get( )->auto_check( c_config::get( )->i[ "rage_quickpeek_enabled" ], c_config::get( )->i[ "rage_quickpeek_enabled_style" ] ) )
	{
		g_Render->RadialGradient3D( position, 23.5f, Color( c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 0 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 1 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 2 ], 15 ), Color( c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 0 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 1 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 2 ], 0 ), false );

		if ( math::world_to_screen( local_origin, localorign ) && math::world_to_screen( position, quic ) )
			g_Render->RadialGradient3D( position, 23.5f, Color( c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 0 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 1 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 2 ], ( 15 / 15 ) * 200 ), Color( c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 0 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 1 ], c_config::get( )->c[ "rage_quick_peek_assist_col" ][ 2 ], 0 ), false );
	}

}
