#include "Menu.h"
#include "MenuFramework/Framework.h"
#include "../hooks/hooks.hpp"
#include "MenuFramework/Renderer.h"
#include "../includes.hpp"
#include "../Configuration/Config.h"
//#include "../Configuration/Config.cpp"
#include "LazyImport.hpp"
#include <filesystem>
#include "../constchars.h"
#include <iostream>
#include <direct.h>
#include "../cheats/visuals/other_esp.h"
#include "../xorstr.hpp"
#include "../nSkinz/parser/parser.hpp"
using namespace IdaLovesMe;
namespace fs = std::filesystem;


std::unordered_map<std::string, int> i;

void CMenu::Initialize( )
{
	if ( this->m_bInitialized )
		return;
	ui::CreateContext( );
	c_config::get( )->refresh_configs( );
	GuiContext* g = Gui_Ctx;
	g->MenuAlpha = 255; //set window opened bla bla
	g->IsClicking = false;
	g->DontMove = false;
	c_config::get( )->load_defaults( );
	this->m_nCurrentTab = 0;
	this->set_menu_opened( true );
	this->m_bInitialized = true;

	auto cfg = c_config::get( );

	cfg->i[ "rage_active_weapon_cfg" ] = 0;

}

bool to_bool( std::string const& s ) {
	return s != "0";
}

std::string to_string( bool s ) {
	if ( s )
		return "1";
	else
		return "0";
}

void draw_lua_items( std::string tab, std::string container ) {
	auto cfg = c_config::get( );
	GuiContext* g = Gui_Ctx;
	static std::string rande;
	for ( auto i : c_lua::get( ).menu_items[ tab ][ container ] )
	{
		auto type = i.type;
		GuiWindow* LuaWindow2 = ui::GetCurrentWindow( );
		LuaWindow2->Disabled = false;
		switch ( type )
		{
			case MENUITEM_CHECKBOX:
				if ( cfg->LuaVisible[ i.key ] ) {
					//if (ui::Checkbox(i.label.c_str(), &cfg->b[i.key)])) {
					if ( ui::Checkbox( i.label.c_str( ), &cfg->b[ i.key.c_str( ) ] ) ) {
						if ( i.callback != sol::nil )
							i.callback( cfg->b[ i.key ] );
					}
				}
				break;
			case MENUITEM_SLIDERINT:
				if ( cfg->LuaVisible[ i.key ] ) {
					ui::SliderInt( i.label.c_str( ), &cfg->i[ i.key ], i.i_min, i.i_max, i.format.c_str( ), cfg->b[ i.key.c_str( ) ] );
					if ( i.callback != sol::nil )
						i.callback( cfg->i[ i.key ] );
				}
				break;
			case MENUITEM_SLIDERFLOAT:
				if ( cfg->LuaVisible[ i.key ] ) {
					ui::SliderFloat( i.label.c_str( ), &cfg->f[ i.key ], i.f_min, i.f_max, "%.f", 1 );
					if ( i.callback != sol::nil )
						i.callback( cfg->f[ i.key ] );

				}
				break;
			case MENUITEM_LISTBOX:
				if ( cfg->LuaVisible[ i.key ] ) {
					static char lname22[ 128 ];
					ui::InputText( "that freaky shit", lname22, true, Flags_None );
					ui::ListBox( i.label.c_str( ), Vec2( 163, 234 ), true, Flags_None, D3DCOLOR_RGBA( 35, 35, 35, g->MenuAlpha ) );
					for ( auto s : i.listitems )
					{
						auto search = std::string( lname22 );
						auto name = std::string( s.c_str( ) );
						std::transform( search.begin( ), search.end( ), search.begin( ), ::tolower );
						std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
						if ( search != "" && name.find( search ) == std::string::npos )
							continue;

						if ( ui::ListBoxSelectableElement( s.c_str( ), rande == s, cfg->b[ s ], Vec2( 0, 0 ) ) ) {
							rande = s;

							c_config::get( )->s[ i.key ] = s;

							if ( i.callback != sol::nil )
								i.callback( s );
						}
					}
					ui::EndListBox( );
				}
				break;
			case MENUITEM_TEXTBOX:
				if ( cfg->LuaVisible[ i.key ] ) {
					static char lname22[ 128 ];
					ui::InputText( i.label.c_str( ), lname22, true, Flags_None );
					if ( i.callback != sol::nil )
						i.callback( cfg->f[ i.key ] );
				}
				break;
			case MENUITEM_KEYBIND:
				if ( cfg->LuaVisible[ i.key ] ) {
					if ( ui::Keybind( i.label.c_str( ), &cfg->i[ i.key ], &cfg->i[ i.key + "_style" ] ) ) {
						if ( i.callback != sol::nil )
							i.callback( cfg->i[ i.key ], cfg->i[ i.key + "_style" ] );
					}
				}
				break;
			case MENUITEM_TEXT:
				if ( cfg->LuaVisible[ i.key ] ) {
					ui::Label( i.label.c_str( ) );
				}
				break;
			case MENUITEM_SINGLESELECT:
				if ( cfg->LuaVisible[ i.key ] ) {
					if ( ui::SingelSelect( i.label.c_str( ), &cfg->i[ i.key ], i.items ) ) {
						if ( i.callback != sol::nil )
							i.callback( cfg->i[ i.key ] );
					}
				}
				break;
			case MENUITEM_MULTISELECT:
				if ( cfg->LuaVisible[ i.key ] ) {
					if ( ui::MultiSelectOld( i.label.c_str( ), &cfg->m[ i.key ], i.items ) ) {
						if ( i.callback != sol::nil )
							i.callback( cfg->m[ i.key ] );
					}
				}

				break;
			case MENUITEM_COLORPICKER:
				if ( cfg->LuaVisible[ i.key ] ) {
					if ( ui::ColorPicker( i.label.c_str( ), cfg->c[ i.key ], Flags_ColorPicker ) ) {
						if ( i.callback != sol::nil )
							i.callback( cfg->c[ i.key ][ 0 ] * 255, cfg->c[ i.key ][ 1 ] * 255, cfg->c[ i.key ][ 2 ] * 255, cfg->c[ i.key ][ 3 ] * 255 );
					}
				}

				break;
			case MENUITEM_BUTTON:
				if ( cfg->LuaVisible[ i.key ] ) {
					if ( ui::Button( i.label.c_str( ) ) ) {
						cfg->b[ i.key ] = true;
					}
					else {
						cfg->b[ i.key ] = false;
					}
				}
				break;
			default:
				break;
		}
	}
}

int h = 0;
void DrawIndicator( std::string name, D3DCOLOR col ) {

	if ( !g_ctx.local( )->is_alive( ) ) //-V807
		return;
	static int width, height;
	m_engine( )->GetScreenSize( width, height );

	Render::Draw->Gradient( Vec2( 14, height - 340 - h - 3 ), Vec2( Render::Draw->GetTextSize( Render::Fonts::Indicator, name.c_str( ) ).x / 2, 33 ), D3DCOLOR_RGBA( 5, 5, 5, 0 ), D3DCOLOR_RGBA( 0, 0, 0, get_a( col ) / 2 ) );
	Render::Draw->Gradient( Vec2( 14 + Render::Draw->GetTextSize( Render::Fonts::Indicator, name.c_str( ) ).x / 2, height - 340 - h - 3 ), Vec2( Render::Draw->GetTextSize( Render::Fonts::Indicator, name.c_str( ) ).x / 2, 33 ), D3DCOLOR_RGBA( 5, 5, 5, get_a( col ) / 2 ), D3DCOLOR_RGBA( 5, 5, 5, 0 ) );

	Render::Draw->Text( name.c_str( ), 24, height - 340 - h + 1, LEFT, Render::Fonts::Indicator, false, D3DCOLOR_RGBA( 5, 5, 5, get_a( col ) / 2 ) );
	Render::Draw->Text( name.c_str( ), 23, height - 340 - h, LEFT, Render::Fonts::Indicator, false, col );
}

void CMenu::Draw( ) {
	//do menu alpha shit then set g->MenuAlpha to your shit 
	//copy from gideon lazy to add
	GuiContext* g = Gui_Ctx;

	static float alpha = 0;
	float fc = m_globals( )->m_frametime * 255 * 10;
	if ( !this->m_bIsOpened && alpha > 0.f )
		alpha = std::clamp( alpha - fc / 255.f, 0.f, 1.f );

	if ( this->m_bIsOpened && alpha < 1.f )
		alpha = std::clamp( alpha + fc / 255.f, 0.f, 1.f );

	g->MenuAlpha = static_cast< int >( floor( alpha * 255 ) );

	if ( !this->m_bIsOpened && alpha == 0.f )
		return;

	if ( m_engine( )->IsTakingScreenshot( ) && c_config::get( )->b[ "hide_from_obs" ] )
		return;

	ui::GetInputFromWindow( "Counter-Strike: Global Offensive" );


	Vec2 MenuSize;


	switch ( CMenu::get( )->DPI( ) ) {
		case 0:
		{
			MenuSize = Vec2( 660, 560 );
			break;
		}
		case 1:
		{
			MenuSize = Vec2( 724, 660 );
			break;
		}
		case 2:
		{
			MenuSize = Vec2( 990, 880 );
			break;
		}
		case 3:
		{
			MenuSize = Vec2( 1155, 1020 );
			break;
		}
		case 4:
		{
			MenuSize = Vec2( 1320, 1030 );
			break;
		}
	}

	ui::SetNextWindowSize( MenuSize );
	ui::Begin( "Main", Flags_None );

	ui::TabButton( xorstr_( "A" ), &this->m_nCurrentTab, 0, 7 ); //rage
	ui::TabButton( xorstr_( "G" ), &this->m_nCurrentTab, 1, 7 ); //legit
	ui::TabButton( xorstr_( "B" ), &this->m_nCurrentTab, 2, 7 ); //antiaim
	ui::TabButton( xorstr_( "D" ), &this->m_nCurrentTab, 3, 7 );//visuals
	ui::TabButton( xorstr_( "D" ), &this->m_nCurrentTab, 4, 7 ); //skinćhanger
	ui::TabButton( xorstr_( "E" ), &this->m_nCurrentTab, 5, 7 ); //cfgs
	ui::TabButton( xorstr_( "H" ), &this->m_nCurrentTab, 6, 7 ); //plist ui::TabButton(xorstr_("F"), &this->m_nCurrentTab, 6, 7); //plist
	ui::TabButton( xorstr_( "H" ), &this->m_nCurrentTab, 7, 7 ); //cfg

	auto dpi = c_config::get( )->i[ "dpi_scale" ];

	if ( ui::GetWindowSize( ).y > 630.f && dpi == 0 )
	{
		ui::TabButton( xorstr_( "I" ), &this->m_nCurrentTab, 8, 7 ); //lua
	}
	if ( ui::GetWindowSize( ).y > 630.f && dpi == 1 )
	{
		ui::TabButtonLua125( xorstr_( "I" ), &this->m_nCurrentTab, 8, 7 ); //lua
	}
	if ( ui::GetWindowSize( ).y > 630.f && dpi == 2 )
	{
		ui::TabButtonLua150( xorstr_( "I" ), &this->m_nCurrentTab, 8, 7 ); //lua
	}




	//ui::TabButton("I", &this->m_nCurrentTab, 8, 7); 
	auto cfg2 = CMenu::get( );
	auto cfg = c_config::get( );
	if ( this->m_nCurrentTab == 0 ) {
	


		GuiWindow* main_wnd = ui::GetCurrentWindow();

		ui::SetNextWindowPos(main_wnd->Pos + Vec2(100, 30));	
		ui::SetNextWindowSize(Vec2(((main_wnd->Size.x - 125) / 2) - 9, 50));
		ui::BeginChildRagewep( "Weapon type", { Vec2( 0, 0 ), Vec2( 3, 0 ) }, Flags_NoResize | Flags_NoMove );
		auto okkk = cfg->i["weapon_select"];
	//	ui::SingleSelectIcon( "whatever", &cfg->i[ "rage_active_weapon_cfg" ], { "Global", "SCAR", "SSG 08", "AWP", "R8 Revolver", "Desert Eagle","Pistol", "Zeus", "Sex" }, Render::Fonts::LBIcons3 );
		ui::GetCurrentWindow()->CursorPos.y = ui::GetCurrentWindow()->Pos.y + 14;
		ui::SingleSelectIcon("Rage_Weapon_Type", &cfg->i["rage_active_weapon_cfg"], &cfg->m["rage_active_weapon_cfg"], { "Global", "G3SG1 / SCAR-20", "SSG 08", "AWP", "R8 Revolver", "Desert Eagle","Pistol", "Zeus" }, Render::Fonts::LBIcons3);

		ui::EndChild( );
		draw_lua_items( "rage", "weapon type" );




		ui::SetNextWindowPos(main_wnd->Pos + Vec2(100, 100));
		ui::SetNextWindowSize(Vec2(((main_wnd->Size.x - 125) / 2) - 9, main_wnd->Size.y - 125));
		ui::BeginChildRagewep("Aimbot", { Vec2(0, 2), Vec2(3, 8) }, Flags_None);
		GuiWindow* Window = ui::GetCurrentWindow( );
		ui::Checkbox( "Enabled", &cfg->b[ "rage_enabled" ] );
		ui::Keybind( "RageKey", &cfg->i[ "rage_key_enabled" ], &cfg->i[ "rage_key_enabled_st" ] );
		ui::SingelSelect( "Target selection", &cfg->i[ "rage_selection" ], { "Cycle", "Cycle (2x)", "Near crosshair", "Highest damage", "Lowest ping", "Best K/D ratio", "Best hit chance" } );
		ui::MultiSelectOld( "Target hitbox", &cfg->m[ "rage_hitbox" ], { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" } );
		ui::MultiSelectOld( "Multi-point", &cfg->m[ "rage_mphitbox" ], { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" } );
		ui::Keybind( "Multipointkey", &cfg->i[ "mp_key_enabled" ], &cfg->i[ "mp_key_enabled_style" ] );
		if ( cfg->m[ "rage_mphitbox" ][ 0 ] || cfg->m[ "rage_mphitbox" ][ 1 ] || cfg->m[ "rage_mphitbox" ][ 2 ] || cfg->m[ "rage_mphitbox" ][ 3 ] || cfg->m[ "rage_mphitbox" ][ 4 ] ) {
			ui::SingelSelectNoText( "Multipoint amn", &cfg->i[ "rage_selectiodasdasdn" ], { "Low", "Medium", "High" } );
			ui::SliderInt( "Multi-point scale", &cfg->i[ "mp_scale" ], 0, 100, "%d%%" );
		}
		ui::SliderInt( "Minimum hit chance", &cfg->i[ "min_hit_chan" ], 0, 100, "%d%%" );
		ui::SliderIntDMG( "Minimum damage", &cfg->i[ "min_hit_dam" ], 0, 120, "%d" );

		ui::Checkbox( "Minimum damage override", &cfg->b[ "override_dmg_bool" ] );

		if ( cfg->b[ "override_dmg_bool" ] )
		{
			ui::Keybind( "Minimum damage override key", &cfg->i[ "min_dmg_override_key" ], &cfg->i[ "min_dmg_override_key_style" ] );
			ui::SliderIntDMG( "Minimum damage override", &cfg->i[ "min_dmg_override" ], 0, 120, "%d" );
		}
		ui::Checkbox( "Prefer safe point", &cfg->b[ "rage_safepoint" ] );
		ui::Label( "Force safe point" );
		ui::Keybind( "Force safe point", &cfg->i[ "rage_sp_enabled" ], &cfg->i[ "rage_sp_enabled_style" ] );
		ui::MultiSelectOld( "Avoid unsafe hitboxes", &cfg->m[ "rage_unsfhitbox" ], { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" } );

		ui::Checkbox( "Prefer body aim", &cfg->b[ "rage_preferbaim" ] ); //rage_safepoint
		ui::Label( "Force body aim" );
		ui::Keybind( "Force baim key", &cfg->i[ "rage_baim_enabled" ], &cfg->i[ "rage_baim_enabled_style" ] );

		ui::Checkbox( "Quick stop", &cfg->b[ "rage_quickstop" ] );
		if ( cfg->b[ "rage_quickstop" ] ) {
			ui::MultiSelectOld( "Quick stop options", &cfg->m[ "rage_quickstop_options" ], { "Early", "Slow motion", "Duck", "Fake duck", "Move between shots", "Ignore molotov", "Taser" } );
		}
		ui::Keybind( "Quick stop key", &cfg->i[ "rage_quickstop_enabled" ], &cfg->i[ "rage_quickstop_enabled_style" ] );


		ui::CheckboxUntrusted( "Double tap", &cfg->b[ "rage_dt" ] );
		ui::Keybind( "dtkey", &cfg->i[ "rage_dt_key" ], &cfg->i[ "rage_dt_key_style" ] );
		if ( cfg->b[ "rage_dt" ] ) {
			ui::SingelSelect( "Double tap mode", &cfg->i[ "rage_dtmode" ], { "Defensive", "Offensive" } );
			ui::SliderInt( "Double tap hit chance", &cfg->i[ "rage_dthc" ], 0, 100, "%d%%" );
			ui::SliderInt( "Double tap fake lag limit", &cfg->i[ "rage_dtfl" ], 0, 15, "%d" );
			ui::MultiSelectOld( "Double tap quick stop", &cfg->m[ "rage_dtqs" ], { "Slow motion", "Move between shots", "Fake-duck" } );
		}

		draw_lua_items( "rage", "aimbot" );
		ui::EndChild( );

		ui::BeginChild( "Other", { Vec2( 6, 0 ), Vec2( 3, 10 ) } );
		GuiWindow* Window3 = ui::GetCurrentWindow( );
		ui::SingelSelect( "Accuracy boost", &cfg->i[ "rage_backtrack" ], { "Off", "Low", "Medium", "High", "Maximum" } );
		ui::Checkbox( "Anti-aim correction", &cfg->b[ "rage_resolver" ] );
		ui::Checkbox( "Automatic fire", &cfg->b[ "auto_fire" ] );
		ui::Checkbox( "Automatic penetration", &cfg->b[ "auto_pen" ] );
		ui::Checkbox( "Silent aim", &cfg->b[ "rage_silent_enabled" ] );
		ui::Checkbox( "Remove recoil", &cfg->b[ "rage_remove_recoil" ] );


		ui::Checkbox( "Delay shot", &cfg->b[ "rage_delay_shot" ] );
		ui::Checkbox( "Quick peek assist", &cfg->b[ "rage_quick_peek_assist" ] );
		ui::Keybind( "Quick peek assist key", &cfg->i[ "rage_quickpeek_enabled" ], &cfg->i[ "rage_quickpeek_enabled_style" ] );
		if ( cfg->b[ "rage_quick_peek_assist" ] ) {
			ui::MultiSelectOld( "Quick peek assist mode", &cfg->m[ "rage_quick_peek_assist" ], { "Retreat on shot", "Retreat on key" } );
			ui::ColorPicker( "quickpeek", cfg->c[ "rage_quick_peek_assist_col" ], Flags_ColorPicker );
			ui::SliderInt( "Quick peek assist distance", &cfg->i[ "rage_qpd" ], 0, 1000, "%d°" );
		}
		ui::Label( "Duck peek assist" );
		ui::Keybind( "Duck peek assist key", &cfg->i[ "rage_fd_enabled" ], &cfg->i[ "rage_fd_enabled_style" ] );
		ui::Checkbox( "Reduce aim step", &cfg->b[ "rage_aim_step" ] );
		ui::SliderInt( "Maximum FOV", &cfg->i[ "rage_fov" ], 1, 180, "%d°" );

		ui::Checkbox( "Log misses due to spread", &cfg->b[ "rage_logs" ] );
		ui::MultiSelectOld( "Low FPS mitigations", &cfg->m[ "rage_fps_miti" ], { "Force low accuracy boost", "Disable multipoint: feet", "Disable multipoint: arms", "Disable multipoint: legs", "Disable hitbox: feet", "Force low multipoint", "Lower hit-chance precision", "Limit targets per tick" } );



		draw_lua_items( "rage", "other" );
		ui::EndChild( );
	}

	if ( this->m_nCurrentTab == 1 ) {
		ui::BeginChild( "Anti-aimbot angle", { Vec2( 0, 0 ), Vec2( 3, 10 ) } );
		GuiWindow* wind1 = ui::GetCurrentWindow( );
		if ( aatab == true )
		{
			ui::Checkbox( "Enabled", &cfg->b[ "aa_enabled" ] );
			ui::SingelSelect( "Pitch", &cfg->i[ "aa_pitch" ], { "Off", "Down", "Minimal", "Up", "Flick up", "Random", "Custom" } );

			if ( cfg->i[ "aa_pitch" ] == 7 )
				ui::SliderIntNoText( "Custom", &cfg->i[ "custom_pitch" ], -89, 89, "%d°", 0 );

			ui::SingelSelect( "Yaw base", &cfg->i[ "aa_yawbase" ], { "Local view", "At targets" } );
			ui::SingelSelect( "Yaw", &cfg->i[ "aa_yaw" ], { "Off", "180", "Spin","Static", "180 Z", "Crosshair" } );


			if ( cfg->i[ "aa_yaw" ] )
				ui::SliderIntNoText( "aayaw", &cfg->i[ "aa_yaw_val" ], -180, 180, "%d", 0 );

			if ( cfg->i[ "aa_yaw" ] != 0 )
			{
				ui::SingelSelect( "Yaw jitter", &cfg->i[ "aa_yawjit" ], { "Off", "Offset", "Center", "Skitter" } );
				if ( cfg->i[ "aa_yawjit" ] )
					ui::SliderIntNoText( "aayawjit", &cfg->i[ "aa_yaw_jit" ], -180, 180, "%d", 0 );
			}

			ui::SingelSelect( "Body yaw", &cfg->i[ "aa_body_yaw" ], { "Off", "Opposite", "Jitter", "Static" } );
			if ( cfg->i[ "aa_body_yaw" ] == 3 )
				ui::SliderInt( "Body yaw value", &cfg->i[ "aa_body_yaw_value" ], -60, 60 );
			ui::Checkbox( "Edge yaw", &cfg->b[ "aa_edgeyaw" ] );
			ui::Checkbox( "Freestanding", &cfg->b[ "aa_fs" ] );
			ui::Keybind( "feestand", &cfg->i[ "freestand_key" ], &cfg->i[ "freestand_key_style" ] );
			ui::SliderInt( "Roll", &cfg->i[ "rollaa_value" ], -50, 50, "%d°", 0 );
		}
		draw_lua_items( "aa", "antiaim" );
		ui::EndChild( );

		ui::BeginChild( "Fake lag", { Vec2( 6, 0 ), Vec2( 3, 5 ) } );
		GuiWindow* wind2 = ui::GetCurrentWindow( );
		ui::Checkbox( "Enabled", &cfg->b[ "fakelag" ] );
		ui::Keybind( "fl_key", &cfg->i[ "fakelag_key" ], &cfg->i[ "fakelag_key_style" ] );
		ui::SingelSelect( "Mode", &cfg->i[ "fakelag_mode" ], { "Dynamic", "Maximum", "Fluctuate" } );
		ui::SliderInt( "Variance", &cfg->i[ "fl_variance" ], 0, 100, "%d%%" );
		ui::SliderInt( "Limit", &cfg->i[ "fl_limit" ], 0, 14, "%d" );
		draw_lua_items( "aa", "fakelag" );
		ui::EndChild( );

		ui::BeginChild( "Others", { Vec2( 6, 7 ), Vec2( 3, 3 ) } );
		GuiWindow* wind3 = ui::GetCurrentWindow( );
		ui::Checkbox( "Slow motion", &cfg->b[ "slowwalk" ] );
		ui::Keybind( "slowwalkkey", &cfg->i[ "slowwalk_key" ], &cfg->i[ "slowwalk_key_style" ] );
		ui::Checkbox( "0 pitch on land", &cfg->b[ "0pitchland" ] );
		ui::SingelSelect( "Leg movement", &cfg->i[ "leg_movementtype" ], { "Off", "Slide", "Never slide" } );
		ui::CheckboxUntrusted( "On shot anti-aim", &cfg->b[ "hideshots" ] );
		ui::Keybind( "hideshots", &cfg->i[ "hs_key" ], &cfg->i[ "hideshots_key_style" ] );
		ui::CheckboxUntrusted( "Fake peek", &cfg->b[ "fakepeek" ] );
		ui::Keybind( "fakepeekkey", &cfg->i[ "fp_key" ], &cfg->i[ "fp_key_style" ] );
		draw_lua_items( "aa", "others" );
		ui::EndChild( );

	}

	if ( this->m_nCurrentTab == 2 ) {
		ui::BeginChild( "Weap0n type", { Vec2( 0, 0 ), Vec2( 9, 0 ) }, Flags_NoResize | Flags_NoMove );

		ui::LegitButton( "G", &this->legitsel, 0, 6, GuiFlags_LegitTab );
		ui::LegitButton( "P", &this->legitsel, 1, 6, GuiFlags_LegitTab );
		ui::LegitButton( "W", &this->legitsel, 2, 6, GuiFlags_LegitTab );
		ui::LegitButton( "d", &this->legitsel, 3, 6, GuiFlags_LegitTab );
		ui::LegitButton( "f", &this->legitsel, 4, 6, GuiFlags_LegitTab );
		ui::LegitButton( "a", &this->legitsel, 5, 6, GuiFlags_LegitTab );

		std::string legit_weapon_preset = std::to_string( cfg->i[ "legit_weapon_preset" ] );
		ui::EndChild( );

		const char* locktime = "%d ms";
		ui::BeginChild( "Legitbot", { Vec2( 0, 2 ), Vec2( 3, 8 ) } );
		{
			ui::Checkbox( "Enabled", &cfg->b[ "legit_enabled" ] );
			ui::Keybind( "legitbind", &cfg->i[ "lb_key" ], &cfg->i[ "lb_key_style" ] );
			ui::SliderFloat( "Speed", &cfg->f[ "legitspeed" ], 0.5, 1, "%.2f", 0 );
			ui::SliderFloat( "Speed (in attack)", &cfg->f[ "legitspeedatk" ], 0.5, 1, "%.2f", 0 );
			ui::SliderInt( "Speed scale - FOV", &cfg->i[ "legit_speedscale" ], 0, 100, "%d%%", 0 );

			if ( cfg->i[ "legit_max_lockontime" ] == 200 )
				locktime = "\u221E";

			ui::SliderInt( "Maximum lock-on time", &cfg->i[ "legit_max_lockontime" ], 0, 200, locktime );
			ui::SliderInt( "Reaction time", &cfg->i[ "legit_reactiontime" ], 0, 200, "%d ms" );
			ui::SliderFloat( "Maximum FOV", &cfg->f[ "legit_maxfov" ], 0, 60, "%.2f°", 1 );
			ui::Label( "Recoil compensation (P/Y)" );
			ui::SliderIntNoText( "legit_recoilcomp1", &cfg->i[ "legit_recoilcomp1" ], 0, 100, "%d%%", 0 );
			ui::SliderIntNoText( "legit_recoilcomp2", &cfg->i[ "legit_recoilcomp2" ], 0, 100, "%d%%", 0 );
			ui::Checkbox( "Quick stop", &cfg->b[ "legit_quickstop" ] );
			ui::Checkbox( "Aim through smoke", &cfg->b[ "legit_smokecheck" ] );
			ui::Checkbox( "Aim if blind", &cfg->b[ "legit_flashcheck" ] );
			ui::Checkbox( "Head", &cfg->b[ "legit_head_hb" ] );
			ui::Checkbox( "Chest", &cfg->b[ "legit_chest_hb" ] );
			ui::Checkbox( "Stomach", &cfg->b[ "legit_stomach_hb" ] );

			draw_lua_items( "legit", "aimbot" );
		}
		ui::EndChild( );

		ui::BeginChild( "Triggerbot", { Vec2( 6, 2 ), Vec2( 3, 5 ) } );
		{
			ui::Checkbox( "Enabled", &cfg->b[ "trigger_enabled" ] );
			ui::Keybind( "triggerbind", &cfg->i[ "tg_key" ], &cfg->i[ "tg_key_style" ] );
			ui::SliderInt( "Minimum hit chance", &cfg->i[ "trigger_hitchance" ], 0, 100, "%d" );
			ui::SliderInt( "Reaction time", &cfg->i[ "trigger_reactiontime" ], 0, 400, "%d ms" );
			ui::Checkbox( "Burst fire", &cfg->b[ "tb_burst" ] );
			ui::SliderInt( "Minimum damage", &cfg->i[ "trigger_mindamage" ], 1, 100, "%d" );
			ui::Checkbox( "Automatic penetration", &cfg->b[ "trigger_penetration" ] );
			ui::Checkbox( "Shoot through smoke", &cfg->b[ "trigger_smokecheck" ] );
			ui::Checkbox( "Shoot if blind", &cfg->b[ "trigger_flashcheck" ] );
			ui::Checkbox( "Head", &cfg->b[ "trigger_head_hb" ] );
			ui::Checkbox( "Chest", &cfg->b[ "trigger_chest_hb" ] );
			ui::Checkbox( "Stomach", &cfg->b[ "trigger_stomach_hb" ] );
			draw_lua_items( "legit", "triggerbot" );
		}
		ui::EndChild( );

		ui::BeginChild( "0ther", { Vec2( 6, 9 ), Vec2( 3, 1 ) } );
		{
			ui::SingelSelect( "Accuracy boost", &cfg->i[ "accuracyboost" ], { "Off", "Low", "Medium", "High", "Maximum" } );
			ui::Checkbox( "Standalone recoil compensation", &cfg->b[ "stand_recoilcomp" ] );
			draw_lua_items( "legit", "other" );
		}
		ui::EndChild( );

	}

	if ( this->m_nCurrentTab == 3 ) {
		ui::BeginChild( "Player ESP", { Vec2( 0, 0 ), Vec2( 3, 6 ) } );
		ui::Label( "Activation type" );
		ui::Keybind( "act_kb", &cfg->i[ "esp_en" ], &cfg->i[ "esp_en_type" ] );
		ui::Checkbox( "Teammates", &cfg->b[ "teammates" ] );
		ui::Checkbox( "Dormant", &cfg->b[ "dormant" ] );
		ui::Checkbox( "Bounding box", &cfg->b[ "bounding" ] );
		ui::ColorPicker( "boundingcol", cfg->c[ "boundingcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Health bar", &cfg->b[ "healbar" ] );
		if ( cfg->b[ "healbar" ] )
		{
			ui::Checkbox( "Health bar override color", &cfg->b[ "healbar_custom_col" ] );
			if ( cfg->b[ "healbar_custom_col" ] )
				ui::ColorPicker( "healbarcolll", cfg->c[ "healbarcoll" ], Flags_ColorPicker );
		}
		ui::Checkbox( "Name", &cfg->b[ "name" ] );
		ui::ColorPicker( "namecol", cfg->c[ "namecoll" ], Flags_ColorPicker );
		ui::Checkbox( "Flags", &cfg->b[ "flags" ] );
		ui::Checkbox( "Weapon text", &cfg->b[ "weptext" ] );
		ui::Checkbox( "Weapon icon", &cfg->b[ "wepicon" ] );
		ui::ColorPicker( "wepcol", cfg->c[ "wepcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Ammo", &cfg->b[ "ammo" ] );
		ui::ColorPicker( "ammocol", cfg->c[ "ammocoll" ], Flags_ColorPicker );
		ui::Checkbox( "Distance", &cfg->b[ "distance" ] );
		ui::Checkbox( "Glow", &cfg->b[ "glow" ] );
		ui::ColorPicker( "glowcol", cfg->c[ "glowcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Hit marker", &cfg->b[ "hitmarker" ] );
		ui::Checkbox( "Hit marker sound", &cfg->b[ "hitsound" ] );
		ui::Checkboxinvisible( "STATIC LEGS IN AIR", &cfg->b[ "legsinair" ] );
		ui::Checkboxinvisible( "moonwalk", &cfg->b[ "moonwalk" ] );
		ui::Checkboxinvisible( "back leg", &cfg->b[ "backwardleg" ] );
		ui::Checkbox( "Visualize aimbot", &cfg->b[ "visaimbot" ] );
		ui::ColorPicker( "visabcol", cfg->c[ "visabcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Visualize aimbot (safe point)", &cfg->b[ "visaimbotsp" ] );
		ui::ColorPicker( "visabspcol", cfg->c[ "visabspcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Visualize sounds", &cfg->b[ "vissounds" ] );
		ui::ColorPicker( "vissoundcol", cfg->c[ "vissoundcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Line of sight", &cfg->b[ "los" ] );
		ui::ColorPicker( "lineofsightcol", cfg->c[ "loscoll" ], Flags_ColorPicker );
		ui::Checkbox( "Money", &cfg->b[ "money" ] );
		ui::Checkbox( "Skeleton", &cfg->b[ "skeleton" ] );
		ui::ColorPicker( "Skeleton", cfg->c[ "skelecol" ], Flags_ColorPicker );
		ui::Checkbox( "Out of FOV arrow", &cfg->b[ "oof" ] );
		ui::ColorPicker( "oofcolo", cfg->c[ "oofcoll" ], Flags_ColorPicker );

		if ( cfg->b[ "oof" ] ) {
			ui::SliderInt( "Size", &cfg->i[ "oofsize" ], 5, 20, "%dpx", 5 );
			ui::SliderInt( "Radius", &cfg->i[ "oofradius" ], 0, 100, "%d%%" );
		}
		draw_lua_items( "visuals", "player esp" );
		ui::EndChild( );

		ui::BeginChild( "Other ESP", { Vec2( 6, 0 ), Vec2( 3, 3 ) } );
		ui::Checkbox( "Radar", &cfg->b[ "radar" ] );
		ui::MultiSelectOld( "Dropped weapons", &cfg->m[ "drpsweapons" ], { "Icon", "Text", "Glow","Ammo", "Distance" } );
		ui::ColorPicker( "dropwepcol", cfg->c[ "dropwepcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Grenades", &cfg->b[ "grenades" ] );
		ui::ColorPicker( "grencol", cfg->c[ "grencoll" ], Flags_ColorPicker );
		ui::Checkbox( "Inaccuracy overlay", &cfg->b[ "innaccover" ] );
		ui::ColorPicker( "inaccol", cfg->c[ "inaccoll" ], Flags_ColorPicker );
		ui::Checkbox( "Recoil overlay", &cfg->b[ "recover" ] );
		ui::Checkbox( "Crosshair", &cfg->b[ "crosshair" ] );
		ui::Checkbox( "Bomb", &cfg->b[ "bomb" ] );
		ui::ColorPicker( "bombcol", cfg->c[ "bomcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Grenade trajectory", &cfg->b[ "grenadetraj" ] );
		ui::ColorPicker( "grentrajcol", cfg->c[ "grentrajcoll" ], Flags_ColorPicker );
		if ( cfg->b[ "grenadetraj" ] ) {
			ui::Label( "Grenade trajectory(hit)" );
			ui::ColorPicker( "hitcolor", cfg->c[ "grenade_hitcolor" ], Flags_ColorPicker );
		}
		ui::Checkbox( "Grenade proximity warning", &cfg->b[ "grenprox" ] );
		ui::Checkbox( "Spectators", &cfg->b[ "spectators" ] );
		ui::Checkbox( "Penetration recticle", &cfg->b[ "penrect" ] );
		ui::Checkbox( "Hostages", &cfg->b[ "hostages" ] );
		ui::MultiSelectOld( "Feature indicators", &cfg->m[ "ftind" ], { "Force safe point", "Force body aim", "Ping spike", "Double tap", "Duck peek assist", "Freestanding", "On shot anti-aim", "Minimum damage override" } );
		ui::Checkbox( "Shared ESP", &cfg->b[ "sharedesp" ] );
		ui::Keybind( "shareesp", &cfg->i[ "shareeeesp_key" ], &cfg->i[ "sharedesp_key_style" ] );
		ui::Checkbox( "Upgrade tablet", &cfg->b[ "tablet" ] );
		ui::Checkbox( "Danger Zone items", &cfg->b[ "dangerzone" ] );
		draw_lua_items( "visuals", "other esp" );
		ui::EndChild( );

		ui::BeginChild( "Colored Models", { Vec2( 0, 8 ), Vec2( 3, 2 ) } );
		ui::Checkbox( "Player", &cfg->b[ "player" ] );
		ui::ColorPicker( "playerecol", cfg->c[ "playercoll" ], Flags_ColorPicker );
		ui::Checkbox( "Player behind wall", &cfg->b[ "player_xqz" ] );
		ui::ColorPicker( "xqzecol", cfg->c[ "xqzcoll" ], Flags_ColorPicker );
		if ( cfg->b[ "player" ] ) {
			ui::SingelSelectNoText( "plyr_Xqz", &cfg->i[ "plyr_xqzzz" ], { "Default", "Solid","Shaded","Metallic", "Glow", "Bubble", "Original" } );
		}

		ui::Checkbox( "Teammate", &cfg->b[ "teammate" ] );
		ui::ColorPicker( "teamcol", cfg->c[ "teamcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Teammate behind wall", &cfg->b[ "teammate_xqz" ] );
		ui::ColorPicker( "txqzcol", cfg->c[ "txqzcoll" ], Flags_ColorPicker );
		if ( cfg->b[ "teammate" ] ) {
			ui::SingelSelectNoText( "teammate_Xqz", &cfg->i[ "teammate_xqzzz" ], { "Default", "Solid","Shaded","Metallic", "Glow", "Bubble", "Original" } );
		}
		ui::Checkbox( "Local player", &cfg->b[ "localplayer" ] );
		ui::ColorPicker( "lpcol", cfg->c[ "lpcoll" ], Flags_ColorPicker );
		if ( cfg->b[ "localplayer" ] ) {
			ui::SingelSelectNoText( "localmaterial", &cfg->i[ "localmaterial" ], { "Default", "Solid","Shaded","Metallic", "Glow", "Buble", "Original" } );

			if ( cfg->i[ "localmaterial" ] == 4 )
			{
				ui::ColorPicker( "ldcol", cfg->c[ "localdoublecolor" ], Flags_ColorPicker );
			}
		}
		ui::MultiSelectOld( "Local player transparency", &cfg->m[ "lptrans" ], { "Grenades", "Scope" } );
		ui::Checkbox( "Local player fake", &cfg->b[ "localplayerfake" ] );
		if ( cfg->b[ "localplayerfake" ] )
			ui::SingelSelectNoText( "fakematerial", &cfg->i[ "fakematerial" ], { "Default", "Solid","Shaded","Metallic", "Glow", "Buble", "Original" } );
		ui::Checkbox( "On shot", &cfg->b[ "onshot" ] );
		ui::ColorPicker( "lpfcol", cfg->c[ "lpfcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Ragdolls", &cfg->b[ "ragdolls" ] );
		ui::Checkbox( "Hands", &cfg->b[ "hands" ] );
		ui::ColorPicker( "handscol", cfg->c[ "handscoll" ], Flags_ColorPicker );
		if ( cfg->b[ "hands" ] ) {
			ui::SingelSelectNoText( "handchams", &cfg->i[ "handchams" ], { "Default", "Solid","Shaded","Metallic", "Glow", "Bubble", "Original" } );
		}


		ui::Checkbox( "Weapon viewmodel", &cfg->b[ "wepvm" ] );
		ui::ColorPicker( "weppvmcol", cfg->c[ "weppvmcoll" ], Flags_ColorPicker );
		if ( cfg->b[ "wepvm" ] ) {
			ui::SingelSelectNoText( "weaponchams", &cfg->i[ "weaponchams" ], { "Default", "Solid","Shaded","Metallic", "Glow", "Bubble", "Original", "Wireframe" } );
		}


		ui::Checkbox( "Disable model occlusion", &cfg->b[ "disoccu" ] );
		ui::Checkbox( "Shadow", &cfg->b[ "shadow" ] );
		ui::ColorPicker( "shadowcol", cfg->c[ "shadowcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Props", &cfg->b[ "props" ] );
		ui::ColorPicker( "propcol", cfg->c[ "propecoll" ], Flags_ColorPicker );
		draw_lua_items( "visuals", "colored models" );
		ui::EndChild( );

		ui::BeginChild( "Effects", { Vec2( 6, 5 ), Vec2( 3, 5 ) } );
		ui::Checkbox( "Remove flashbang effects", &cfg->b[ "remflash" ] );
		ui::Checkbox( "Remove smoke grenades", &cfg->b[ "remsmoke" ] );
		ui::Checkbox( "Remove fog", &cfg->b[ "remfog" ] );
		ui::Checkbox( "Remove skybox", &cfg->b[ "remsky" ] );
		ui::SingelSelect( "Visual recoil adjustment", &cfg->i[ "visrecolad" ], { "Off", "On" } );
		ui::SliderInt( "Transparent walls", &cfg->i[ "transwalls" ], 0, 100, "%d%%" );
		ui::SliderInt( "Transparent props", &cfg->i[ "transprops" ], 0, 100, "%d%%" );
		ui::MultiSelectOld( "Brightness adjustment", &cfg->m[ "brightadj" ], { "Fullbright", "Nightmode" } );
		if ( cfg->m[ "brightadj" ][ 1 ] ) {
			ui::ColorPicker( "adjcol", cfg->c[ "adjcol" ], Flags_ColorPicker );
		}
		ui::Checkbox( "Remove scope overlay", &cfg->b[ "remscope" ] );
		ui::Checkbox( "Instant scope", &cfg->b[ "instascope" ] );
		ui::Checkbox( "Disable post processing", &cfg->b[ "dispostpro" ] );
		ui::Checkbox( "Force third person (alive)", &cfg->b[ "ftp" ] );
		ui::Keybind( "ftp", &cfg->i[ "ftp_key" ], &cfg->i[ "ftp_key_style" ] );
		ui::Checkbox( "Force third person (dead)", &cfg->b[ "ftp2" ] );
		ui::Checkbox( "Disable rendering of teammates", &cfg->b[ "distmrend" ] );
		ui::Checkbox( "Bullet tracers", &cfg->b[ "tracers" ] );
		ui::ColorPicker( "bullcol", cfg->c[ "bullcoll" ], Flags_ColorPicker );
		ui::Checkbox( "Bullet impacts", &cfg->b[ "impacts" ] );
		ui::Checkbox( "Override skybox", &cfg->b[ "skyboxx" ] );
		ui::ColorPicker( "overrideskyboxcol", cfg->c[ "skycol" ], Flags_ColorPicker );
		if ( cfg->b[ "skyboxx" ] ) {
			ui::SingelSelect( "Custom skybox", &cfg->i[ "skybox" ], { "cs_tibet", "cs_baggage_skybox_", "italy", "jungle","office" } );
		}
		draw_lua_items( "visuals", "effects" );
		ui::EndChild( );
	}

	if ( this->m_nCurrentTab == 4 ) {
		ui::BeginChild( "Miscellaneous", { Vec2( 0, 0 ), Vec2( 3, 10 ) } );
		ui::SliderInt( "Override FOV", &cfg->i[ "misc_fov_over" ], 1, 135, "%d°" );
		ui::SliderInt( "Override zoom FOV", &cfg->i[ "misc_over_zoom_fov" ], 0, 100, "%d%%" );
		ui::Checkbox( "Knifebot", &cfg->b[ "misc_Knifebot" ] );
		if ( cfg->b[ "misc_Knifebot" ] ) {
			ui::MultiSelectOld( "Knifebot selection", &cfg->m[ "fsda" ], { "Swing", "Full stab" } );
		}
		ui::Checkbox( "Zeusbot", &cfg->b[ "misc_zeusbot" ] );
		ui::Checkbox( "Automatic weapons", &cfg->b[ "misc_autowep" ] );
		ui::Checkbox( "Reveal compettive ranks", &cfg->b[ "misc_rev_comp" ] );
		ui::Checkbox( "Reveal Overwatch players", &cfg->b[ "misc_rev_overwatch" ] );
		ui::Checkbox( "Auto-accept matchmaking", &cfg->b[ "misc_auto_accept" ] );
		ui::Checkbox( "Clan tag spammer", &cfg->b[ "misc_clantag" ] );
		ui::Checkbox( "Log weapon purchases", &cfg->b[ "misc_log_wep" ] );
		ui::Checkbox( "Log damage dealt", &cfg->b[ "misc_log_dam" ] );
		ui::Checkbox( "Automatic grenade release", &cfg->b[ "misc_grenade_release" ] );
		ui::Keybind( "autogrenade key", &cfg->i[ "auto_grenade_key" ], &cfg->i[ "auto_grenade_key_style" ] );
		if ( cfg->b[ "misc_grenade_release" ] )
			ui::SliderInt( "", &cfg->i[ "misc_age_val" ], 0, 100, "%dms" );
		ui::Checkbox( "Ping spike", &cfg->b[ "misc_ping_spike" ] );
		ui::Keybind( "Ping spike key", &cfg->i[ "ping_spike_key" ], &cfg->i[ "ping_spike_key_style" ] );
		if ( cfg->b[ "misc_ping_spike" ] )
			ui::SliderInt( "", &cfg->i[ "misc_pingspike_val" ], 0, 200, "%dms" );
		ui::Label( "Freelook" );
		ui::Keybind( "free look key", &cfg->i[ "free_look_key" ], &cfg->i[ "free_look_key_style" ] );
		ui::Checkbox( "Persistent kill feed", &cfg->b[ "misc_pers_feed" ] );
		ui::Label( "Last second defuse" );
		ui::Keybind( "last sec defuse", &cfg->i[ "last_sec_defuse" ], &cfg->i[ "last_sec_defuse_key_style" ] );
		ui::Checkbox( "Disable sv_pure", &cfg->b[ "disable_sv_pure" ] );
		ui::Checkbox( "Unlock hidden cvars", &cfg->b[ "misc_hidden_cvars" ] );
		ui::Button( "Steal player name" );
		ui::Button( "Dump MM wins" );
		draw_lua_items( "misc", "misc" );
		ui::EndChild( );

		ui::BeginChild( "Movement", { Vec2( 6, 0 ), Vec2( 3, 4 ) } );
		ui::Checkbox( "Standalone quick stop", &cfg->b[ "stand_quick_stop" ] );
		ui::CheckboxUntrusted( "Infinite duck", &cfg->b[ "inf_duck" ] );
		//ui::Checkbox( "Info Bar", &cfg->b[ "info_bar" ] );
		ui::Checkbox( "Easy strafe", &cfg->b[ "ez_strafe" ] );
		ui::Checkbox( "Bunny hop", &cfg->b[ "bhop" ] );
		ui::Checkbox( "Air strafe", &cfg->b[ "air_starfe" ] );
		if ( cfg->b[ "air_starfe" ] )
		{
			ui::MultiSelectOld( "Air strafe mode", &cfg->m[ "strafermode" ], { "Viewangles", "Movement keys" } );
			ui::SliderInt( "Air strafe smoothing", &cfg->i[ "strafer_smoothing" ], 0, 100, "%d%%" );
			ui::Checkbox( "Avoid collisions", &cfg->b[ "avoidcoll" ] );
			ui::Checkbox( "Z-hop", &cfg->b[ "zhoppoop" ] );
			ui::Keybind( "zhopkey", &cfg->i[ "zhoppoop_key" ], &cfg->i[ "zhoppoop_key_style" ] );
			ui::Checkbox( "Pre-speed", &cfg->b[ "prespeed" ] );
		}



		ui::Checkbox( "No fall damage", &cfg->b[ "no fall" ] );
		static int cols[ 4 ] = { 255,255,255,255 };
		ui::ColorPicker( "asdijasidjij", cols, Flags_ColorPicker );
		ui::SingelSelect( "Air duck", &cfg->i[ "airducksel" ], { "Off", "On", "Spam", "Fake", "On land" } );
		ui::Checkbox( "Block bot", &cfg->b[ "blockbot" ] );
		ui::Keybind( "blockkeybind", &cfg->i[ "bb_key" ], &cfg->i[ "bb_key_style" ] );
		ui::Checkbox( "Jump at edge", &cfg->b[ "edgejump" ] );
		ui::Keybind( "ej", &cfg->i[ "ej_key" ], &cfg->i[ "ej_key_style" ] );
		ui::Checkbox( "Fast walk", &cfg->b[ "fwalk" ] );
		draw_lua_items( "misc", "movement" );
		ui::EndChild( );

		ui::BeginChild( "Settings", { Vec2( 6, 6 ), Vec2( 3, 4 ) } );
		ui::Label( "Menu key" );
		static int nigga;
		ui::Keybind( "menu_key", &cfg->i[ "misc_menukey" ], &nigga );
		ui::Label( "Menu color" );

		//ui::ColorPicker("##menu_key", this->ThemeColor, Flags_ColorPicker);
		ui::ColorPicker( "##menu_key", cfg->c[ "menucolor" ], Flags_ColorPicker );

		ui::SingelSelect( "DPI scale", &cfg->i[ "dpi_scale" ], { "100%", "125%", "150%", "175%", "200%" } );

		ui::Checkbox( "Anti-untrusted", &cfg->b[ "anti_untrusted" ] );
		ui::Checkbox( "Hide from OBS", &cfg->b[ "hide_from_obs" ] );
		ui::Checkbox( "Low FPS warning", &cfg->b[ "fps_warning" ] );
		ui::Checkbox( "Lock menu layout", &this->LockMenu );
		if ( ui::Button( "Reset menu layout" ) )
			this->ResetMenu = true;
		if ( ui::Button( "Unload" ) )
			this->unload = true;

		//exit(0);



		//if (this->PopUpOpen)
		draw_lua_items( "misc", "settings" );
		ui::EndChild( );


	}




	if ( this->m_nCurrentTab == 5 ) {
		auto& selected_entry = g_cfg.skins.skinChanger[ current_profile ];
		selected_entry.itemIdIndex = current_profile;

		ui::BeginChild( "Model options", { Vec2( 0, 0 ), Vec2( 3, 10 ) } );
		ui::Checkbox( "Knife changer", &cfg->b[ "skins_knifechanger" ] );
		if ( cfg->b[ "skins_knifechanger" ] )
			( ui::SingelSelectNoText( "Knife", &cfg->i[ "skins_knifechanger_items" ],
				{
				"Default",
				"Bayonet knife",
				"M9 Bayonet knife",
				"Karambit knife",
				"Bowie Knife",
				"Butterfly Knife",
				"Falchion knife",
				"Flip knife",
				"Gut Knife",
				"Huntsman Knife",
				"Shadow Daggers knifes",
				"Navaja Knife",
				"Stiletto knife",
				"Talon Knife",
				"Urus Knife",
				"Nomad Knife",
				"Skeleton Knife",
				"Survival Knife",
				"Paracord Knife",
				"CSS Knife",
				} ) );



		ui::Checkbox( "Glove changer", &cfg->b[ "skins_glovechanger" ] );

		if ( cfg->b[ "skins_glovechanger" ] ) {
			( ui::SingelSelectNoText( "CT", &cfg->i[ "skin_changer_model_glove_ct" ],
				{
				"Default",
				"Bloodhound",
				"Sporty",
				"Slick",
				"Leathery",
				"Moto",
				"Motorcycle",
				"Specialist",
				"Bloodhound Hydra",
				"Glove Anarchist",
				} ) );
			( ui::SingelSelectNoText( "T", &cfg->i[ "skin_changer_model_glove_t" ],
				{
				"Default",
				"Bloodhound",
				"Sporty",
				"Slick",
				"Leathery",
				"Moto",
				"Motorcycle",
				"Specialist",
				"Bloodhound Hydra",
				"Glove Anarchist",
				} ) );
		}

		ui::Checkbox( "Mask changer", &cfg->b[ "skins_maskchanger" ] );
		if ( cfg->b[ "skins_maskchanger" ] )
			( ui::SingelSelectNoText( "Mask", &cfg->i[ "skin_changer_model_mask" ],
				{
				"Default",
				"Dallas",
				"Battle Mask",
				"Evil Clown",
				"Boar",
				"Anaglyph",
				"Bunny",
				"Chains",
				"Chicken",
				"Devil Plastic",
				"Pumpkin",
				"Skull",
				"Wolf",
				"Doll",
				"Samurai"
				} ) );

		ui::Checkbox( "Agent changer", &cfg->b[ "skins_agentchanger" ] );
		if ( cfg->b[ "skins_agentchanger" ] )
			( ui::SingelSelectNoText( "Agent changer", &cfg->i[ "skin_changer_model_agent" ], {
				"Jumpsuit A",
				"Jumpsuit B",
				"Jumpsuit C",
				"Anarchist A",
				"Anarchist B",
				"Anarchist C",
				"Separatist A",
				"Separatist B",
				//"Separatist C",
				//"Separatist D",
				//"CTM. FBI",
				//"CTM. FBI A",
				//"CTM. FBI B",
				//"CTM. FBI C",
				//"CTM. FBI E",
				//"Gign Model A",
				//"Gign Model B",
				//"Gign Model C",
				//"CTM. ST6",
				//"CTM. ST6 A",
				//"CTM. ST6 B",
				//"CTM. ST6 C",
				//"CTM. ST6 D",
				//"CTM. IDF B",
				//"CTM. IDF C",
				//"CTM. IDF D",
				//"CTM. IDF E",
				//"CTM. IDF F",
				//"CTM. Swat",
				//"TM. Swat A",
				//"CTM. Swat B",
				//"CTM. Swat C",
				//"CTM. Swat D",
				//"CTM. Sas",
				//"CTM. Gsg9",
				//"CTM. Gsg9 A",
				//"CTM. Gsg9 B",
				//"CTM. Gsg9 C",
				//"CTM. Gsg9 D",
				//"Professional A",
				//"Professional B",
				//"Professional C",
				//"Professional D",
				//"Leet A",
				//"Leet B",
				//"Leet C",
				//"Leet D",
				//"Balkan A",
				//"Balkan B",
				//"Balkan C",
				//"Balkan D",
				//"Pirate A",
				//"Pirate B",
				//"Pirate C",
				} ) );
		ui::EndChild( );

		static int currentskin = 0;
		ui::BeginChild( "Weapon skin", { Vec2( 6, 0 ), Vec2( 3, 10 ) } );
		ui::Checkbox( "Enabled", &cfg->b[ "skinchanger_enabled" ] );

		if ( cfg->b[ "skinchanger_enabled" ] )
		{
			ui::Checkbox( "StatTrak", &cfg->b[ "sc_stattrak" ] );
			ui::SliderInt( "Quality", &cfg->i[ "sc_quality" ], 0, 100, "%d%%" );
			ui::SliderInt( "Seed", &cfg->i[ "sc_seed" ], 0, 1001, "%d" );
			ui::Checkbox( "Filter by weapon", &cfg->b[ "filter_by_wep" ] );
			static char lnameer2[ 999999 ] = "\0";

			std::vector <std::string> xyi;

			for ( auto player : SkinChanger::skinKits )
				xyi.emplace_back( player.name );

			int autooust = 0;

			ui::InputText( "_", c_config::get( )->Skins_input_text, true, Flags_None );
			ui::ListBox( "Skins", Vec2( 163, 306 ), false, Flags_None );

			for ( auto s : xyi ) {
				auto search = std::string( c_config::get( )->Skins_input_text );
				auto name = std::string( s.c_str( ) );
				std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
				std::transform( search.begin( ), search.end( ), search.begin( ), ::tolower );

				if ( search != "" && name.find( search ) == std::string::npos )
					continue;

				if ( ui::ListBoxSelectableConfig( s.c_str( ), currentskin == autooust, Vec2( 0, 0 ) ) ) {

					currentskin = autooust;
				}
				autooust++;

				CMenu::index_knife = SkinChanger::skinKits.at( currentskin ).id;


			}

			ui::EndListBox( );
		}
		ui::EndChild( );
	}

	auto player = players_section;

	static std::vector <Player_list_data> players;

	if ( !g_cfg.player_list.refreshing )
	{
		players.clear( );

		for ( auto player : g_cfg.player_list.players )
			players.emplace_back( player );
	}

	static int currentplayer = 0;

	if ( this->m_nCurrentTab == 6 ) {
		ui::BeginChild( "Players", { Vec2( 0, 0 ), Vec2( 3, 10 ) } );
		if ( !m_engine( )->IsConnected( ) ) {
			ui::ListBox( "PlayerSelc", Vec2( 163, 425 ), false, Flags_None, D3DCOLOR_RGBA( 27, 27, 27, 255 ) );

			ui::EndListBox( );
			ui::ButtonDisabled( "Reset all" );
			ui::EndChild( );

			ui::BeginChild( "Adjustments", { Vec2( 6, 0 ), Vec2( 3, 10 ) } );
			ui::CheckboxDisabled( "Add to whitelist", &cfg->b[ "plr_wl" ] );
			ui::CheckboxDisabled( "Allow shared ESP updates", &cfg->b[ "plr_sharedesp" ] );
			ui::CheckboxDisabled( "Disable visuals", &cfg->b[ "plr_disablevis" ] );
			ui::CheckboxDisabled( "High priority", &cfg->b[ "plr_prio" ] );
			ui::CheckboxDisabled( "Force pitch", &cfg->b[ "plr_forcepitch" ] );
			ui::CheckboxDisabled( "Force body yaw", &cfg->b[ "plr_forceyaw" ] );
			ui::CheckboxDisabled( "Correction active", &cfg->b[ "plr_resolver" ] );
			ui::SingelSelectDisabled( "Override prefer body aim", &cfg->i[ "orrderverplrbaim" ], { "-", "Override prefer body aim" } );
			ui::SingelSelectDisabled( "Override safe point", &cfg->i[ "orrderverplrbaasdasdim" ], { "-", "Override safe point" } );
			ui::ButtonDisabled( "Apply to all" );
		}
		else {
			std::vector <std::string> player_names;

			for ( auto player : players )
				player_names.emplace_back( player.name );

			int autooust = 0;

			ui::ListBox( "PlayerSelc", Vec2( 163, 425 ), false, Flags_None, D3DCOLOR_RGBA( 27, 27, 27, 255 ) );
			for ( auto s : player_names ) {
				if ( ui::ListBoxSelectableConfig( s.c_str( ), currentplayer == autooust, Vec2( 0, 0 ) ) ) {
					currentplayer = autooust;
				}
				autooust++;
			}

			ui::EndListBox( );
			ui::Button( "Reset all" );
			ui::EndChild( );

			ui::BeginChild( "Adjustments", { Vec2( 6, 0 ), Vec2( 3, 10 ) } );
			ui::Checkbox( "Add to whitelist", &g_cfg.player_list.white_list[ players.at( currentplayer ).i ] );
			ui::Checkbox( "Allow shared ESP updates", &cfg->b[ "plr_sharedesp" ] );
			ui::Checkbox( "Disable visuals", &g_cfg.player_list.disable_visuals[ players.at( currentplayer ).i ]/*&cfg->b["plr_disablevis"]*/ );
			ui::Checkbox( "High priority", &g_cfg.player_list.high_priority[ players.at( currentplayer ).i ] );
			ui::Checkbox( "Force pitch", &cfg->b[ "plr_forcepitch" ] );
			ui::Checkbox( "Force body yaw", &cfg->b[ "plr_forceyaw" ] );
			ui::Checkbox( "Correction active", &cfg->b[ "plr_resolver" ] );

			ui::Checkbox( "Force safe points", &g_cfg.player_list.force_safe_points[ players.at( currentplayer ).i ] );
			ui::SingelSelect( "Override prefer body aim", &cfg->i[ "orrderverplrbaim" ], { "-", "Override prefer body aim" } );
			ui::SingelSelect( "Override safe point", &cfg->i[ "orrderverplrsafe" ], { "-", "Override safe point" } );
			ui::Button( "Apply to all" );
		}
		ui::EndChild( );
	}

	if ( this->m_nCurrentTab == 7 ) {
		ui::BeginChild( "Presets", { Vec2( 0, 0 ), Vec2( 3, 10 ) } );

		ui::ListBox( "Presettsss", Vec2( 163, 126 ), false, Flags_None, D3DCOLOR_RGBA( 35, 35, 35, g->MenuAlpha ) );
		static std::string randomvar;
		static char lname[ 999999 ] = "\0";
		for ( auto& s : c_config::get( )->configs ) {
			std::string st = s.substr( 0, s.size( ) - 4 );
			if ( ui::ListBoxSelectableConfig( st.c_str( ), randomvar == s, Vec2( 0, 0 ) ) ) {
				randomvar = s;

				strcpy_s( c_config::get( )->Config_input_text, st.c_str( ) );
				configID = c_config::get( )->get_config_id( s );
				c_config::get( )->refresh_configs( );

				cfg->i[ "_preset" ] = configID;
			}
		}
		ui::EndListBox( );

		ui::Keybind( "Niggaconfigs", &cfg->i[ "configuration_key" ], &cfg->i[ "configuration_key_style" ] );
		ui::InputText( "ConfigText", c_config::get( )->Config_input_text, false, Flags_None );
		if ( ui::Button( "Load" ) ) {
			if ( !( std::string( c_config::get( )->Config_input_text ) == "" ) ) {
				c_config::get( )->load( );

			}
		}
		if ( ui::Button( "Save" ) ) {
			if ( !( std::string( c_config::get( )->Config_input_text ) == "" ) )
			{
				c_config::get( )->save( );
				randomvar = std::string( c_config::get( )->Config_input_text ) + ".ini";
				c_config::get( )->refresh_configs( );
				configID = std::distance( c_config::get( )->configs.begin( ), std::find( c_config::get( )->configs.begin( ), c_config::get( )->configs.end( ), randomvar ) );
			}
		}
		if ( ui::Button( "Delete" ) ) {
			if ( !( configID > c_config::get( )->configs.size( ) ) ) {
				c_config::get( )->remove( configID );
				configID = -1;
				if ( c_config::get( )->configs.size( ) < 1 || std::distance( c_config::get( )->configs.begin( ), std::find( c_config::get( )->configs.begin( ), c_config::get( )->configs.end( ), randomvar ) ) > -1 ) {
					memset( c_config::get( )->Config_input_text, 0, sizeof c_config::get( )->Config_input_text );
				}
			}
		}

		if ( ui::Button( "Reset" ) ) {
			c_config::get( )->load_defaults( );
		}

		if ( ui::Button( "Import from clipboard" ) ) {
			c_config::get( )->import( );
			c_config::get( )->refresh_configs( );
		}
		if ( ui::Button( "Export to clipboard" ) ) {
			c_config::get( )->export_to_clipboard( );
		}

		draw_lua_items( "configs", "presets" );
		ui::EndChild( );

		ui::BeginChild( "Lua", { Vec2( 6, 0 ), Vec2( 3, 10 ) } );

		ui::Checkbox( "Enabled", &cfg->b[ "lua_enabled" ] );
		if ( !cfg->b[ "lua_enabled" ] ) {
			ui::CheckboxDisabled( "Allow unsafe scripts", &cfg->b[ "lua_allow_unsafe" ] );
		}
		else
		{
			ui::Checkbox( "Allow unsafe scripts", &cfg->b[ "lua_allow_unsafe" ] );
		}
		if ( !cfg->b[ "lua_enabled" ] ) {
			ui::ButtonDisabled( "Reload active scripts" );
		}
		else
		{
			if ( ui::Button( "Reload active scripts" ) ) {
				c_lua::get( ).reload_all_scripts( );
			}
		}
		c_lua::get( ).refresh_scripts( );
		static char lname2[ 128 ];
		ui::InputText( "LuaText", lname2, true, Flags_None );
		struct stat File_stats;
		static std::string lazy = "Load script";
		static int scriptIDD = -1;
		static int oldscriptid = -1;
		static bool showtimestamp = false;
		time_t s_Last_Time_Edited;
		if ( !cfg->b[ "lua_enabled" ] ) {
			ui::ListBox( "LuaSelc", Vec2( 163, 234 ), true, Flags_None, D3DCOLOR_RGBA( 27, 27, 27, 255 ) );
		}
		else {
			ui::ListBox( "LuaSelc", Vec2( 163, 234 ), true, Flags_None, D3DCOLOR_RGBA( 35, 35, 35, g->MenuAlpha ) );
		}


		GuiWindow* LuaWindow = ui::GetCurrentWindow( );
		if ( !cfg->b[ "lua_enabled" ] ) {
			LuaWindow->Disabled = true;
		}
		else
		{
			LuaWindow->Disabled = false;
		}

		static std::string rand;
		std::string Text_result;
		for ( auto s : c_lua::get( ).scripts_names )
		{

			auto search = std::string( lname2 );
			auto name = std::string( s.c_str( ) );

			std::transform( search.begin( ), search.end( ), search.begin( ), ::tolower );
			std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
			if ( search != "" && name.find( search ) == std::string::npos )
				continue;

			std::string st = s.substr( 0, s.size( ) - 4 );
			if ( ui::ListBoxSelectableLua( st.c_str( ), rand == s, c_lua::get( ).loaded.at( c_lua::get( ).get_script_id( s ) ), Vec2( 0, 0 ) ) ) {
				rand = s;
				scriptIDD = c_lua::get( ).get_script_id( s );
				cfg->i[ "sel_lua" ] = scriptIDD;
				if ( c_lua::get( ).loaded.at( scriptIDD ) ) {
					lazy = "Unload script";
				}
				else {
					lazy = "Load script";
				}

			}
			int swag = 999;
		}
		ui::EndListBox( );
		if ( showtimestamp ) {
			if ( stat( c_lua::get( ).pathes.at( scriptIDD ).string( ).c_str( ), &File_stats ) == 0 )
				s_Last_Time_Edited = File_stats.st_mtime;

			int s_Difference = ( static_cast< int >( time( NULL ) ) - static_cast< int >( s_Last_Time_Edited ) );
			if ( s_Difference < 60 )
				Text_result = "Updated " + std::to_string( s_Difference ) + " seconds ago";
			else if ( s_Difference >= 60 && s_Difference < 3600 )
				Text_result = "Updated " + std::to_string( s_Difference / 60 ) + " minutes ago";
			else if ( s_Difference >= 3600 && s_Difference < 86400 )
				Text_result = "Updated " + std::to_string( s_Difference / 3600 ) + " hours ago";
			else
				Text_result = "Updated " + std::to_string( s_Difference / 86400 ) + " days ago";
		}
		if ( scriptIDD != -1 ) {
			ui::Label( Text_result.c_str( ) );
		}
		char szStartupval[ 64 ] = {};
		//static bool startupshit;
		bool startupshit;
		if ( oldscriptid != scriptIDD ) {
			LI_FN( GetPrivateProfileStringA ).get( )( crypt_str( "autoload" ), crypt_str( rand.c_str( ) ), "", szStartupval, 64, crypt_str( "C:\\gs\\luas\\menu.ini" ) );
			std::string startupval;
			startupval = szStartupval;
			showtimestamp = true;
			startupshit = to_bool( startupval );
		}
		if ( c_lua::get( ).is_cloud_lua( rand ) ) {
			ui::CheckboxDisabled( "Load on startup", &startupshit );
		}
		else {
			ui::CheckboxDisabled( "Load on startup", &startupshit );
		}
		WritePrivateProfileStringA( "autoload", rand.c_str( ), to_string( startupshit ).c_str( ), "C:\\gs\\luas\\menu.ini" );

		if ( cfg->b[ "lua_enabled" ] && scriptIDD != -1 ) {
			if ( ui::Button( lazy.c_str( ) ) ) {
				if ( c_lua::get( ).loaded.at( scriptIDD ) ) {
					c_lua::get( ).unload_script( scriptIDD );
					lazy = "Load script";
				}
				else {
					c_lua::get( ).load_script( scriptIDD );
					lazy = "Unload script";
				}
			}
		}

		draw_lua_items( "configs", "lua" );
		ui::EndChild( );

	}

	if ( this->m_nCurrentTab == 8 ) {
		ui::BeginChild( "A", { Vec2( 0, 0 ), Vec2( 3, 10 ) } );
		GuiWindow* Window = ui::GetCurrentWindow( );

		draw_lua_items( "lua", "a" );
		ui::EndChild( );

		ui::BeginChild( "B", { Vec2( 6, 0 ), Vec2( 3, 10 ) } );
		GuiWindow* Window3 = ui::GetCurrentWindow( );

		draw_lua_items( "lua", "b" );
		//ui::Slider("nigga", slider, 1, 100);
		ui::EndChild( );
	}


	ui::End( );
}



D3DCOLOR CMenu::MenuColor( ) {
	GuiContext* g = Gui_Ctx;
	auto cfg = c_config::get( );
	return D3DCOLOR_RGBA( cfg->c[ "menucolor" ][ 0 ], cfg->c[ "menucolor" ][ 1 ], cfg->c[ "menucolor" ][ 2 ], g->MenuAlpha );
}

int CMenu::DPI( ) {
	return c_config::get( )->i[ "dpi_scale" ];
}

float CMenu::GetDPINum( ) {
	switch ( CMenu::get( )->DPI( ) ) {
		case 0:
		{
			return 1.f;
			break;
		}
		case 1:
		{
			return 1.25f;
			break;
		}
		case 2:
		{
			return 1.50f;
			break;
		}
		case 3:
		{
			return 1.75f;
			break;
		}
		case 4:
		{
			return 2.f;
			break;
		}


	}

	return NULL;
}

int CMenu::GetDPITab( ) {
	switch ( CMenu::get( )->DPI( ) ) {
		case 0:
		{
			return 0;
			break;
		}
		case 1:
		{
			return 10;
			break;
		}
		case 2:
		{
			return 16;
			break;
		}
		case 3:
		{
			return 24;
			break;
		}
		case 4:
		{
			return 32;
			break;
		}


	}

	return NULL;
}



LPD3DXFONT CMenu::GetFontDPI( ) {
	switch ( CMenu::get( )->DPI( ) ) {
		case 0:
		{
			return Render::Fonts::Verdana;
			break;
		}
		case 1:
		{
			return Render::Fonts::Verdana25;
			break;
		}
		case 2:
		{
			return Render::Fonts::Verdana50;
			break;
		}
		case 3:
		{
			return Render::Fonts::Verdana75;
			break;
		}
		case 4:
		{
			return Render::Fonts::Verdana200;
			break;
		}


	}

	return NULL;
}

LPD3DXFONT CMenu::GetFontDPI_small( ) {
	switch ( CMenu::get( )->DPI( ) ) {
		case 0:
		{
			return Render::Fonts::SmallFont;
			break;
		}
		case 1:
		{
			return Render::Fonts::Verdana25;
			break;
		}
		case 2:
		{
			return Render::Fonts::Verdana50;
			break;
		}
		case 3:
		{
			return Render::Fonts::Verdana75;
			break;
		}
		case 4:
		{
			return Render::Fonts::Verdana200;
			break;
		}
	}

	return NULL;
}

LPD3DXFONT CMenu::GetFontBold( ) {
	switch ( CMenu::get( )->DPI( ) ) {
		case 0:
		{
			return Render::Fonts::Tahombd;
			break;
		}
		case 1:
		{
			return Render::Fonts::Tahombd25;
			break;
		}
		case 2:
		{
			return Render::Fonts::Tahombd50;
			break;
		}
		case 3:
		{
			return Render::Fonts::Tahombd75;
			break;
		}
		case 4:
		{
			return Render::Fonts::Tahombd200;
			break;
		}


	}

	return NULL;
}

LPD3DXFONT CMenu::GetKeybindDPI( ) {
	switch ( CMenu::get( )->DPI( ) ) {
		case 0:
		{
			return Render::Fonts::Keybind;
			break;
		}
		case 1:
		{
			return Render::Fonts::Keybind75;
			break;
		}
		case 2:
		{
			return Render::Fonts::Keybind75;
			break;
		}
		case 3:
		{
			return Render::Fonts::Keybind75;
			break;
		}
		case 4:
		{
			return Render::Fonts::Keybind200;
			break;
		}
	}

	return NULL;
}

bool CMenu::is_menu_opened( ) {
	return this->m_bIsOpened;
}

int CMenu::GetTabNumber( ) {
	return this->m_nCurrentTab;
}

void CMenu::set_menu_opened( bool v ) {
	this->m_bIsOpened = v;
}