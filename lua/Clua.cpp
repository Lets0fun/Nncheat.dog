#include "HTTPRequest.hpp"
#include "Clua.h"	
#include <ShlObj_core.h>
#include <Windows.h>
#include <any>
#include "../cheats/visuals/other_esp.h"
#include "..\utils\csgo.hpp"
#include "..\cheats\misc\logs.h"
#include "..\includes.hpp"
#include "../cheats/visuals/player_esp.h"
#include "../Menu/MenuFramework/Framework.h"
#include "../Menu/MenuFramework/Renderer.h"
#pragma comment(lib, "wininet.lib")
#include <Windows.h>
#include <string>
//#include "../cheats/lagcompensation/local_animations.h"
using namespace std;
using namespace IdaLovesMe;

void lua_panic( sol::optional <std::string> message )
{
	if ( !message )
		return;

	auto log = crypt_str( "Lua error: " ) + message.value_or( "unknown" );
	eventlogs::get( ).add( log, false );
}

int extract_owner( sol::this_state st ) {
	sol::state_view lua_state( st );
	sol::table rs = lua_state[ "debug" ][ "getinfo" ]( 2, "S" );
	//99.99999% sure
	std::string source = rs[ "source" ];
	std::string filename = std::filesystem::path( source.substr( 1 ) ).filename( ).string( );
	return c_lua::get( ).get_script_id( filename );
}

std::string get_current_script( sol::this_state s )
{
	sol::state_view lua_state( s );
	sol::table rs = lua_state[ "debug" ][ "getinfo" ]( 2, ( "S" ) );
	std::string source = rs[ "source" ];
	std::string filename = std::filesystem::path( source.substr( 1 ) ).filename( ).string( );

	return filename;
}

int get_current_script_id( sol::this_state s )
{
	return c_lua::get( ).get_script_id( get_current_script( s ) );
}

namespace ns_client
{
	bool is_key_down( int key ) {
		return HIWORD( GetKeyState( key ) );
	}
	void add_event_clantag( const char* text ) {
		auto apply = [ ] ( const char* tag ) -> void
		{
			using Fn = int( __fastcall* )( const char*, const char* );
			static auto fn = reinterpret_cast< Fn >( util::FindSignature( crypt_str( "engine.dll" ), crypt_str( "53 56 57 8B DA 8B F9 FF 15" ) ) );

			fn( tag, tag );
		};
		auto tag = crypt_str( "" );
		tag = crypt_str( text );
		return apply( tag );
	}

	long long get_unix_time( )
	{
		const auto p1 = std::chrono::system_clock::now( );
		auto reuslt = std::chrono::duration_cast< std::chrono::seconds >( p1.time_since_epoch( ) ).count( );

		return reuslt;
	}

	void flag( const Box& box, const char* niga, Color color ) {
		auto _x = box.x + box.w + 3, _y = box.y - 3;
		return render::get( ).text( fonts[ ESP ], _x, _y, color, HFONT_CENTERED_NONE, niga );
	}

	string windowsvers()
	{
		string initstring;
		initstring = "Error : string is null";
		UCHAR szFileSys[255], szVolNameBuff[255];
		DWORD dwSerial;
		DWORD dwMFL;
		DWORD dwSysFlags;
		int error = 0;

		//request information of Volume C, using GetVolumeInformatioA winapi function
		bool fail = GetVolumeInformationA("C:\\", (LPSTR)szVolNameBuff, 255, &dwSerial, &dwMFL, &dwSysFlags, (LPSTR)szFileSys, 255);
		if (!fail) {
			initstring = "Error : Not elevated (please run this with admin rights)";
			return initstring;
		}

		stringstream hwidstream;
		hwidstream << hex << dwSerial; // convert volume serial to hex

		string HWID = hwidstream.str();

		for (int i = 0; i < HWID.length(); i++) // shift converted serial by 6 characters 
			HWID[i] += 6; // (you can change this to shift as much as you want)

		return HWID;

	}

	

	void add_event_callback( sol::this_state s, std::string eventname, sol::protected_function func )
	{
		if ( eventname != crypt_str( "on_paint" ) && eventname != crypt_str( "on_createmove" ) && eventname != crypt_str( "on_shot" ) && eventname != crypt_str( "run_command" ) && eventname != crypt_str( "on_framestage" ) && eventname != crypt_str( "round_prestart" ) && eventname != crypt_str( "aim_fire" ) && eventname != crypt_str( "aim_hit" ) && eventname != crypt_str( "aim_miss" ) && eventname != crypt_str( "shutdown" ) && eventname != crypt_str( "setup_command" ) && eventname != crypt_str( "pre_render" ) && eventname != crypt_str( "on_unload" ) && eventname != crypt_str( "player_hurt" ) && eventname != crypt_str( "aim_miss" ) && eventname != crypt_str( "on_round_start" ) )
		{
			eventlogs::get( ).add( crypt_str( "Lua error: invalid callback \"" ) + eventname + '\"', false );
			return;
		}

		if ( c_lua::get( ).loaded.at( get_current_script_id( s ) ) )//new
			c_lua::get( ).hooks.registerHook( eventname, get_current_script_id( s ), func );
	}

	uint64_t create_interface( const std::string& szModule, const std::string& szSignature )
	{
		return util::FindSignature( szModule.c_str( ), szSignature.c_str( ) );
	}

	void add_callback( sol::this_state s, std::string callback, sol::protected_function func )
	{

		if ( c_lua::get( ).loaded.at( get_current_script_id( s ) ) )//new
			c_lua::get( ).hooks.registerHook( callback, get_current_script_id( s ), func );
	}

	void execute( std::string& command )
	{
		if ( command.empty( ) )
			return;

		m_engine( )->ExecuteClientCmd( command.c_str( ) );
	}

	std::string get_user_name( ) {
		DWORD BufferSize = 20000;
		char szUsername[ 20000 ] = {};
		std::string username;
		RegGetValue( HKEY_CURRENT_USER, "Software\\gsfun", "login", RRF_RT_ANY, NULL, ( PVOID )&szUsername, &BufferSize );
		username = szUsername;
		return username;
	}

	void load_script( std::string name )
	{
		c_lua::get( ).refresh_scripts( );
		c_lua::get( ).load_script( c_lua::get( ).get_script_id( name ) );
	}

	void unload_script( std::string name )
	{
		c_lua::get( ).refresh_scripts( );
		c_lua::get( ).unload_script( c_lua::get( ).get_script_id( name ) );
	}

	void staticairleg( bool ok )
	{
		//	staticlegair = true;
	}


	void log( std::string text )
	{
		eventlogs::get( ).add( text, false );
	}
}

std::vector <std::pair <std::string, menu_item>>::iterator find_item( std::vector <std::pair <std::string, menu_item>>& items, const std::string& name )
{
	for ( auto it = items.begin( ); it != items.end( ); ++it )
		if ( it->first == name )
			return it;

	return items.end( );
}

menu_item find_item( std::vector <std::vector <std::pair <std::string, menu_item>>>& scripts, const std::string& name )
{
	for ( auto& script : scripts )
	{
		for ( auto& item : script )
		{
			std::string item_name;

			auto first_point = false;
			auto second_point = false;

			for ( auto& c : item.first )
			{
				if ( c == '.' )
				{
					if ( first_point )
					{
						second_point = true;
						continue;
					}
					else
					{
						first_point = true;
						continue;
					}
				}

				if ( !second_point )
					continue;

				item_name.push_back( c );
			}

			if ( item_name == name )
				return item.second;
		}
	}

	return menu_item( );
}

namespace ns_menu
{
	std::tuple<int, int> get_menu_pos( ) {
		auto g = IdaLovesMe::Gui_Ctx;
		if ( !g )
			return std::make_tuple( 0, 0 );
		int w, h;
		GuiWindow* Window = ui::GetCurrentWindow( );
		return std::make_tuple( Window->Pos.x, Window->Pos.y );
	}

	std::tuple<int, int> get_menu_size( ) {
		auto g = IdaLovesMe::Gui_Ctx;
		if ( !g )
			return std::make_tuple( 0, 0 );
		int w, h;
		GuiWindow* Window = ui::GetCurrentWindow( );
		return std::make_tuple( Window->Size.x, Window->Size.y );
	}

	bool get_visible( )
	{
		return CMenu::get( )->is_menu_opened( );
	}

	Color get_menu_color( ) {
		auto cfg = c_config::get( ); // cfg->c["menucolor"]
		int r, g, b;
		r = cfg->c[ "menucolor" ][ 0 ];
		g = cfg->c[ "menucolor" ][ 1 ];
		b = cfg->c[ "menucolor" ][ 2 ];

		return Color( r, g, b );
	}

	void set_menu_color( Color color ) {
bruh:
		auto cfg = c_config::get( );
		cfg->c[ "menucolor" ][ 0 ] = color.r( );
		cfg->c[ "menucolor" ][ 1 ] = color.g( );
		cfg->c[ "menucolor" ][ 2 ] = color.b( );
		goto bruh;
	}

	void disable_tab( bool status )
	{
		CMenu::get( )->aatab = status;

	}

	void set_visible( bool visible )
	{
		CMenu::get( )->set_menu_opened( visible );
	}

	bool is_active( std::string key )
	{
		if ( c_config::get( )->auto_check( c_config::get( )->i[ key ], c_config::get( )->i[ key + "_style" ] ) )
			return true;
		else
			return false;
	}


	int get_menu_alpha( ) {
		auto g = IdaLovesMe::Gui_Ctx;
		if ( g )
			return g->MenuAlpha;

		return 255;
	}

	auto next_line_counter = 0;
#include <algorithm>

	void next_line( sol::this_state s )
	{

	}

	std::tuple<sol::object, sol::object, sol::object, sol::object> get( sol::this_state s, std::string key ) {
		auto cfg = c_config::get( );
		std::tuple<sol::object, sol::object, sol::object, sol::object> retn = std::make_tuple( sol::nil, sol::nil, sol::nil, sol::nil );

		for ( auto kv : cfg->b )
			if ( kv.first == key )
				retn = std::make_tuple( sol::make_object( s, kv.second ), sol::nil, sol::nil, sol::nil );

		for ( auto kv : cfg->c )
			if ( kv.first == key )
				retn = std::make_tuple( sol::make_object( s, ( int )( kv.second[ 0 ] * 255 ) ), sol::make_object( s, ( int )( kv.second[ 1 ] * 255 ) ), sol::make_object( s, ( int )( kv.second[ 2 ] * 255 ) ), sol::make_object( s, ( int )( kv.second[ 3 ] * 255 ) ) );

		for ( auto kv : cfg->f )
			if ( kv.first == key )
				retn = std::make_tuple( sol::make_object( s, kv.second ), sol::nil, sol::nil, sol::nil );

		for ( auto kv : cfg->i )
			if ( kv.first == key )
				retn = std::make_tuple( sol::make_object( s, kv.second ), sol::nil, sol::nil, sol::nil );

		for ( auto kv : cfg->m )
			if ( kv.first == key )
				retn = std::make_tuple( sol::make_object( s, kv.second ), sol::nil, sol::nil, sol::nil );

		for ( auto kv : cfg->s )
			if ( kv.first == key )
				retn = std::make_tuple( sol::make_object( s, kv.second ), sol::nil, sol::nil, sol::nil );

		return retn;
	}

	std::string add_check_box( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::optional<bool> def, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_CHECKBOX;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.b_default = def.value_or( false );

		item.callback = cb.value_or( sol::nil );


		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	std::string add_combo_box( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::vector<const char*> items, std::optional<int> def, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_SINGLESELECT;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.i_default = def.value_or( 0 );
		item.items = items;
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	std::string add_list_box( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::vector<std::string> items, std::optional<int> def, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_LISTBOX;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.i_default = def.value_or( 0 );
		item.listitems = items;
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	std::string add_text_box( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_TEXTBOX;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	int add_key_bind( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::optional<int> def, std::optional<sol::function> cb )
	{
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_KEYBIND;

		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.b_default = def.value_or( false );
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return c_config::get( )->i[ key ];
	}

	std::string new_multiselect( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::vector<const char*> items, std::optional<std::map<int, bool>> def, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_MULTISELECT;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.m_default = def.value_or( std::map<int, bool> {} );
		item.items = items;
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}
	std::string add_slider_int( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, int min, int max, std::optional<std::string> format, std::optional<int> def, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_SLIDERINT;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.i_default = def.value_or( false );

		item.callback = cb.value_or( sol::nil );

		printf( "vallvack: ", item.callback );

		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	std::string add_slider_float( sol::this_state s, std::string tab, std::string container, std::string label, std::string key, float min, float max, std::optional<std::string> format, std::optional<float> def, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_SLIDERFLOAT;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		item.f_default = def.value_or( 0.f );
		item.f_min = min;
		item.f_max = max;
		item.format = format.value_or( "%.0f" );
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	std::string add_color_picker( sol::this_state s, std::string tab, std::string container, std::string id, std::string key, std::optional<int> r, std::optional<int> g, std::optional<int> b, std::optional<int> a, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_COLORPICKER;
		item.script = extract_owner( s );
		item.label = id;
		item.key = key;
		item.c_default[ 0 ] = r.value_or( 255 ) / 255.f;
		item.c_default[ 1 ] = g.value_or( 255 ) / 255.f;
		item.c_default[ 2 ] = b.value_or( 255 ) / 255.f;
		item.c_default[ 3 ] = a.value_or( 255 ) / 255.f;
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	std::string new_text( sol::this_state s, std::string tab, std::string container, std::string label, std::string key ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_TEXT;
		item.script = extract_owner( s );
		item.label = label;
		item.key = key;
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;

		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	std::string new_button( sol::this_state s, std::string tab, std::string container, std::string id, std::string key, std::optional<sol::function> cb ) {
		std::transform( tab.begin( ), tab.end( ), tab.begin( ), ::tolower );
		std::transform( container.begin( ), container.end( ), container.begin( ), ::tolower );

		MenuItem_t item;
		item.type = MENUITEM_BUTTON;
		item.script = extract_owner( s );
		item.label = id;
		item.key = key;
		item.callback = cb.value_or( sol::nil );
		c_config::get( )->LuaVisible[ key ] = true;
		if ( tab == "LUA" || tab == "lua" )
			c_lua::get( ).luatabactive = true;
		c_lua::get( ).menu_items[ tab ][ container ].push_back( item );
		return key;
	}

	void set_visibility( std::string key, bool v ) {
		auto cfg = c_config::get( );
		cfg->LuaVisible[ key ] = v;
	}

	std::unordered_map <std::string, bool> first_update;
	std::unordered_map <std::string, MenuItem_t> stored_values;
	std::unordered_map <std::string, void*> config_items;

	void set_bool( std::string key, bool v ) {
		auto cfg = c_config::get( );
		cfg->b[ key ] = v;
	}

	void set_float( std::string key, float v ) {
		auto cfg = c_config::get( );

		if ( ceilf( v ) != v )
			cfg->f[ key ] = v;
		else
			cfg->i[ key ] = ( int )v;
	}

	void set_color( std::string key, int r, int g, int b, int a ) {
		auto cfg = c_config::get( );
		cfg->c[ key ][ 0 ] = r;
		cfg->c[ key ][ 1 ] = g;
		cfg->c[ key ][ 2 ] = b;
		cfg->c[ key ][ 3 ] = a;

	}

	void set_multiselect( std::string key, int pos, bool e ) {
		auto cfg = c_config::get( );
		cfg->m[ key ][ pos ] = e;
	}
}

namespace ns_globals
{
	int get_framerate( )
	{
		return g_ctx.globals.framerate;
	}

	int get_ping( )
	{
		return g_ctx.globals.ping;
	}

	std::string get_server_address( )
	{
		if ( !m_engine( )->IsInGame( ) )
			return "Unknown";

		auto nci = m_engine( )->GetNetChannelInfo( );

		if ( !nci )
			return "Unknown";

		auto server = nci->GetAddress( );

		if ( !strcmp( server, "loopback" ) )
			server = "Local server";
		else if ( m_gamerules( )->m_bIsValveDS( ) )
			server = "Valve server";

		return server;
	}

	std::string get_time( )
	{
		return g_ctx.globals.time;
	}

	std::string get_username( )
	{
		return g_ctx.username;
	}

	float get_realtime( )
	{
		return m_globals( )->m_realtime;
	}

	float get_curtime( )
	{
		return m_globals( )->m_curtime;
	}

	float get_frametime( )
	{
		return m_globals( )->m_frametime;
	}

	int get_tickcount( )
	{
		return m_globals( )->m_tickcount;
	}

	int get_framecount( )
	{
		return m_globals( )->m_framecount;
	}

	float get_intervalpertick( )
	{
		return m_globals( )->m_intervalpertick;
	}

	int get_maxclients( )
	{
		return m_globals( )->m_maxclients;
	}
}

namespace ns_engine
{
	static int width, height;
	std::tuple<int, int> get_screen_size( ) {
		int w, h;
		m_engine( )->GetScreenSize( w, h );
		return std::make_tuple( w, h );
	}

	player_info_t get_player_info( int i )
	{
		player_info_t player_info;
		m_engine( )->GetPlayerInfo( i, &player_info );

		return player_info;
	}

	int get_player_for_user_id( int i )
	{
		return m_engine( )->GetPlayerForUserID( i );
	}

	int get_local_player_index( )
	{
		return m_engine( )->GetLocalPlayer( );
	}

	Vector get_view_angles( )
	{
		Vector view_angles;
		m_engine( )->GetViewAngles( view_angles );

		return view_angles;
	}

	void set_view_angles( Vector view_angles )
	{
		math::normalize_angles( view_angles );
		m_engine( )->SetViewAngles( view_angles );
	}

	bool is_in_game( )
	{
		return m_engine( )->IsInGame( );
	}

	bool is_connected( )
	{
		return m_engine( )->IsConnected( );
	}

	std::string get_level_name( )
	{
		return m_engine( )->GetLevelName( );
	}

	std::string get_level_name_short( )
	{
		return m_engine( )->GetLevelNameShort( );
	}

	std::string get_map_group_name( )
	{
		return m_engine( )->GetMapGroupName( );
	}

	bool is_playing_demo( )
	{
		return m_engine( )->IsPlayingDemo( );
	}

	bool is_recording_demo( )
	{
		return m_engine( )->IsRecordingDemo( );
	}

	bool is_paused( )
	{
		return m_engine( )->IsPaused( );
	}

	bool is_taking_screenshot( )
	{
		return m_engine( )->IsTakingScreenshot( );
	}

	bool is_hltv( )
	{
		return m_engine( )->IsHLTV( );
	}
}

namespace ns_render
{
	Vector2D get_cursor_pos( )
	{
		int x, y;
		m_surface( )->SurfaceGetCursorPos( x, y );
		return Vector2D( x, y );
	}

	void add_indicator( std::string& text, int r, int g, int b, int a )
	{
		if ( !g_ctx.available( ) )
			return;
		if ( !g_ctx.local( )->is_alive( ) ) //-V807
			return;

		otheresp::get( ).m_indicators.push_back( { text, D3DCOLOR_RGBA( r, g, b, a ) } );
	}

	void add_side_indicator( std::string& text, Color color )
	{
		if ( !g_ctx.available( ) )
			return;

		if ( !g_ctx.local( )->is_alive( ) ) //-V807
			return;

		//playeresp::SideInfo::SideInfo(playeresp::SideInfo( text, D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) ) );
	}

	Vector world_to_screen( const Vector& world )
	{
		Vector screen;

		if ( !math::world_to_screen( world, screen ) )
			return ZERO;

		return screen;
	}

	std::tuple<int, int> get_text_size( vgui::HFont font, const std::string& text )
	{
		return std::make_tuple( render::get( ).text_width( font, text.c_str( ) ), render::get( ).text_height( font, text.c_str( ) ) );

	}

	using namespace Render;

	vgui::HFont create_font( std::string fontname, float w, float h, int blur, int flags )
	{
		g_ctx.last_font_name = fontname;

		auto font = m_surface( )->FontCreate( );
		m_surface( )->SetFontGlyphSet( font, fontname.c_str( ), ( int )w, ( int )h, blur, 0, flags );

		return font;
	}

	IDirect3DTexture9* load_texture( std::string file_path, int w, int h )
	{
		IDirect3DTexture9* skin_image;
		//
		//std::string vpk_path = file_path;

		//const auto handle = m_basefilesys( )->Open( vpk_path.c_str( ), crypt_str( "r" ), crypt_str( "GAME" ) );

		//if ( handle )
		//{
		//	int file_len = m_basefilesys( )->Size( handle );

		//	char* image = new char[ file_len ];

		//	m_basefilesys( )->Read( image, file_len, handle );

		//	m_basefilesys( )->Close( handle );

		//	D3DXCreateTextureFromFileInMemory( Render::Draw->GetD3dDevice( ), image, file_len, &skin_image  );

		//	delete[ ] image;

		//	return skin_image;
		//}

		//return nullptr;

		if ( D3DXCreateTextureFromFile( Render::Draw->GetD3dDevice( ), file_path.c_str( ), &skin_image ) )
		{
			return skin_image;
		}

	}

	void render_texture( LPDIRECT3DTEXTURE9 texture, int x, int y, int w, int h, Color color )
	{
		Render::Draw->Sprite( texture, Vec2( x, y ), Vec2( w, h ), D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}
	void draw_rounded_rect( float x, float y, int w, int h, int points, int radius, Color color )
	{
		render::get( ).rounded_box( ( int )x, ( int )y, w, h, points, radius, color );
	}

	void draw_text( float x, float y, Color color, const std::string& text )
	{
		//render::get().text(font, (int)x, (int)y, color, HFONT_CENTERED_NONE, text.c_str());
		Render::Draw->Text_test( text.c_str( ), x, y, 0, Render::Fonts::Verdana, false, D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}
	void draw_text_small( float x, float y, Color color, const std::string& text )
	{
		//render::get().text(font, (int)x, (int)y, color, HFONT_CENTERED_NONE, text.c_str());
		Render::Draw->Text_test( text.c_str( ), x, y, 0, Render::Fonts ::SmallFont, false, D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}

	void draw_text_centered( vgui::HFont font, float x, float y, Color color, bool centered_x, bool centered_y, const std::string& text )
	{
		DWORD centered_flags = HFONT_CENTERED_NONE;

		if ( centered_x )
		{
			centered_flags &= ~HFONT_CENTERED_NONE; //-V753
			centered_flags |= HFONT_CENTERED_X;
		}

		if ( centered_y )
		{
			centered_flags &= ~HFONT_CENTERED_NONE;
			centered_flags |= HFONT_CENTERED_Y;
		}

		render::get( ).text( font, ( int )x, ( int )y, color, centered_flags, text.c_str( ) );
	}

	void draw_line( float x, float y, float x2, float y2, Color color )
	{
		render::get( ).line( ( int )x, ( int )y, ( int )x2, ( int )y2, Color( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}
	void draw_rect( float x, float y, float w, float h, Color color )
	{
		Render::Draw->Rect( Vec2( x, y ), Vec2( w, h ), 0 , D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}

	void draw_rect_filled( float x, float y, float w, float h, Color color )
	{
		//render::get().rect_filled((int)x, (int)y, (int)w, (int)h, Color(color.r(), color.g(), color.b(), color.a()));

		Render::Draw->FilledRect( Vec2( x, y ), Vec2( w, h ), D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}

	void draw_rect_filled_gradient( float x, float y, float w, float h, Color color, Color color2 )
	{
		//render::get().gradient((int)x, (int)y, (int)w, (int)h, Color(color.r(), color.g(), color.b(), color.a()), Color(color2.r(), color2.g(), color2.b(), color2.a()), GRADIENT_HORIZONTAL);
		Render::Draw->Gradient( Vec2( x, y ), Vec2( w, h ), D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ), D3DCOLOR_RGBA( color2.r( ), color2.g( ), color2.b( ), color2.a( ) ) );
	}

	void draw_circle( float x, float y, float points, float radius, Color color )
	{
		//render::get().circle((int)x, (int)y, (int)points, (int)radius, Color(color.r(), color.g(), color.b(), color.a

	}

	void draw_circle_filled( float x, float y, float points, float radius, Color color )
	{
		render::get( ).circle_filled( ( int )x, ( int )y, ( int )points, ( int )radius, color );
	}

	void draw_triangle( float x, float y, float x2, float y2, float x3, float y3, Color color )
	{
		//	render::get().triangle(Vector2D((int)x, (int)y), Vector2D((int)x2, (int)y2), Vector2D((int)x3, (int)y3), color);
		Render::Draw->Triangle( Vec2( x, y ), Vec2( x2, y2 ), Vec2( x3, y3 ), D3DCOLOR_RGBA( color.r( ), color.g( ), color.b( ), color.a( ) ) );
	}

	void draw_logmsg( float x, float y, std::string log )
	{
		auto logs_size_inverted = 10 - log.size( );
		render::get( ).text( fonts[ LOGS ], x, y + y - logs_size_inverted * 14, Color( 255, 255, 255 ), HFONT_CENTERED_NONE, log.c_str( ) );
	}
}

namespace ns_console
{

	std::unordered_map <std::string, ConVar*> convars;

	bool get_bool( const std::string& convar_name )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return false;
			}
		}

		if ( !convars[ convar_name ] )
			return false;

		return convars[ convar_name ]->GetBool( );
	}

	int get_int( const std::string& convar_name )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return 0;
			}
		}

		if ( !convars[ convar_name ] )
			return 0;

		return convars[ convar_name ]->GetInt( );
	}

	float get_float( const std::string& convar_name )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return 0.0f;
			}
		}

		if ( !convars[ convar_name ] )
			return 0.0f;

		return convars[ convar_name ]->GetFloat( );
	}

	std::string get_string( const std::string& convar_name )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return crypt_str( "" );
			}
		}

		if ( !convars[ convar_name ] )
			return "";

		return convars[ convar_name ]->GetString( );
	}

	void set_bool( const std::string& convar_name, bool value )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return;
			}
		}

		if ( !convars[ convar_name ] )
			return;

		if ( convars[ convar_name ]->GetBool( ) != value )
			convars[ convar_name ]->SetValue( value );
	}

	void set_int( const std::string& convar_name, int value )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return;
			}
		}

		if ( !convars[ convar_name ] )
			return;

		if ( convars[ convar_name ]->GetInt( ) != value )
			convars[ convar_name ]->SetValue( value );
	}

	void set_float( const std::string& convar_name, float value )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return;
			}
		}

		if ( !convars[ convar_name ] )
			return;

		if ( convars[ convar_name ]->GetFloat( ) != value ) //-V550
			convars[ convar_name ]->SetValue( value );
	}

	void set_string( const std::string& convar_name, const std::string& value )
	{
		if ( convars.find( convar_name ) == convars.end( ) )
		{
			convars[ convar_name ] = m_cvar( )->FindVar( convar_name.c_str( ) );

			if ( !convars[ convar_name ] )
			{
				eventlogs::get( ).luaerror( crypt_str( "Lua error: cannot find ConVar \"" ) + convar_name + '\"', false );
				return;
			}
		}

		if ( !convars[ convar_name ] )
			return;

		if ( convars[ convar_name ]->GetString( ) != value )
			convars[ convar_name ]->SetValue( value.c_str( ) );
	}
}

namespace ns_events
{
	void register_event( sol::this_state s, std::string event_name, sol::protected_function function )
	{
		if ( std::find( g_ctx.globals.events.begin( ), g_ctx.globals.events.end( ), event_name ) == g_ctx.globals.events.end( ) )
		{
			m_eventmanager( )->AddListener( &hooks::hooked_events, event_name.c_str( ), false );
			g_ctx.globals.events.emplace_back( event_name );
		}

		c_lua::get( ).events[ get_current_script_id( s ) ][ event_name ] = function;
	}
}

namespace ns_entitylist
{
	sol::optional <player_t*> get_local_player( )
	{
		if ( !m_engine( )->IsInGame( ) )
			return sol::optional <player_t*>( sol::nullopt );

		return ( player_t* )m_entitylist( )->GetClientEntity( m_engine( )->GetLocalPlayer( ) );
	}

	sol::optional <player_t*> get_player_by_index( int i )
	{
		if ( !m_engine( )->IsInGame( ) )
			return sol::optional <player_t*>( sol::nullopt );

		return ( player_t* )m_entitylist( )->GetClientEntity( i );
	}

	sol::optional <int> get_local_player_index( )
	{
		if ( !m_engine( )->IsInGame( ) )
			return sol::optional <int>( sol::nullopt );

		return ( int )m_engine( )->GetLocalPlayer( );
	}

	sol::optional <weapon_t*> get_weapon_by_player( sol::optional <player_t*> player )
	{
		if ( !m_engine( )->IsInGame( ) )
			return sol::optional <weapon_t*>( sol::nullopt );

		if ( !player.has_value( ) )
			return sol::optional <weapon_t*>( sol::nullopt );

		return player.value( )->m_hActiveWeapon( ).Get( );
	}

}

namespace ns_cmd
{
	bool get_send_packet( )
	{
		if ( !g_ctx.get_command( ) )
			return true;

		return g_ctx.send_packet;
	}

	void set_send_packet( bool send_packet )
	{
		if ( !g_ctx.get_command( ) )
			return;

		g_ctx.send_packet = send_packet;
	}

	int get_choke( )
	{
		if ( !g_ctx.get_command( ) )
			return 0;

		return m_clientstate( )->iChokedCommands;
	}

	bool get_button_state( int button )
	{
		if ( !g_ctx.get_command( ) )
			return false;

		return g_ctx.get_command( )->m_buttons & button;
	}

	void set_button_state( int button, bool state )
	{
		if ( !g_ctx.get_command( ) )
			return;

		if ( state )
			g_ctx.get_command( )->m_buttons |= button;
		else
			g_ctx.get_command( )->m_buttons &= ~button;
	}
}

namespace ns_utils
{
	uint64_t find_signature( const std::string& szModule, const std::string& szSignature )
	{
		return util::FindSignature( szModule.c_str( ), szSignature.c_str( ) );
	}

	void* EasyInterface( const char* _Module, const char* _Object )
	{
		ULONG CreateInterface = ( ULONG )GetProcAddress( GetModuleHandleA( _Module ), "CreateInterface" );

		ULONG ShortJump = ( ULONG )CreateInterface + 5;

		ULONG Jump = ( ( ( ULONG )CreateInterface + 5 ) + *( ULONG* )ShortJump ) + 4;

		InterfaceReg* List = **( InterfaceReg*** )( Jump + 6 );

		do {
			if ( List )
			{
				if ( strstr( List->m_pName, _Object ) && ( strlen( List->m_pName ) - strlen( _Object ) ) < 5 )
					return List->m_CreateFn( );
			}

		} while ( List = List->m_pNext );

		return 0;
	}

	void* create_interface( const std::string& dll, const std::string& name )
	{
		return EasyInterface( dll.c_str( ), name.c_str( ) );
	}

}

namespace ns_exploits
{
	void set_rc_time( int value ) {
		g_cfg.lua.tolerance = math::clamp( value, 1, 3 );
	}
	void set_max_shift_value( int value ) {
		g_cfg.lua.override_shift = math::clamp( value, 10, 14 );
	}
}

namespace ns_http //new
{
	std::string get( sol::this_state s, std::string& link )
	{
		if ( !c_config::get( )->b[ "lua_allow_unsafe" ] )
		{
			c_lua::get( ).unload_script( get_current_script_id( s ) );
			eventlogs::get( ).add( crypt_str( "Please, allow unsafe scripts" ), false );
			return "";
		}
		try
		{
			http::Request request( link );

			const http::Response response = request.send( crypt_str( "GET" ) );
			return std::string( response.body.begin( ), response.body.end( ) );
		}
		catch ( const std::exception& e )
		{
			eventlogs::get( ).add( crypt_str( "Request failed, error: " ) + std::string( e.what( ) ), false );
			return "";
		}

	}

	std::string post( sol::this_state s, std::string& link, std::string& params )
	{
		if ( !c_config::get( )->b[ "lua_allow_unsafe" ] )
		{
			c_lua::get( ).unload_script( get_current_script_id( s ) );
			eventlogs::get( ).add( crypt_str( "Please, allow unsafe scripts" ), false );
			return "";
		}
		try
		{
			http::Request request( link );
			const http::Response response = request.send( crypt_str( "POST" ), params, { crypt_str( "Content-Type: application/x-www-form-urlencoded" ) } );
			return std::string( response.body.begin( ), response.body.end( ) );
		}
		catch ( const std::exception& e )
		{
			eventlogs::get( ).add( crypt_str( "Request failed, error: " ) + std::string( e.what( ) ), false );
			return "";
		}
	}
}


namespace ns_file //new
{
	void append( sol::this_state s, std::string& path, std::string& data )
	{
		if ( !c_config::get( )->b[ "lua_allow_unsafe" ] )
		{
			c_lua::get( ).unload_script( get_current_script_id( s ) );
			eventlogs::get( ).add( crypt_str( "Please, allow files read or write" ), false );
			return;
		}

		std::ofstream out( path, std::ios::app | std::ios::binary );

		if ( out.is_open( ) )
			out << data;
		else
			eventlogs::get( ).add( crypt_str( "Can't append to file: " ) + path, false );

		out.close( );
	}
	void write( sol::this_state s, std::string& path, std::string& data )
	{
		if ( !c_config::get( )->b[ "lua_allow_unsafe" ] )
		{
			c_lua::get( ).unload_script( get_current_script_id( s ) );
			eventlogs::get( ).add( crypt_str( "Please, allow files read or write" ), false );
			return;
		}

		std::ofstream out( path, std::ios::binary );

		if ( out.is_open( ) )
			out << data;

		else
			eventlogs::get( ).add( crypt_str( "Can't write to file: " ) + path, false );

		out.close( );
	}
	std::string read( sol::this_state s, std::string& path )
	{
		if ( !c_config::get( )->b[ "lua_allow_unsafe" ] )
		{
			c_lua::get( ).unload_script( get_current_script_id( s ) );
			eventlogs::get( ).add( crypt_str( "Please, allow files read or write" ), false );
			return "";
		}

		std::ifstream file( path, std::ios::binary );

		if ( file.is_open( ) )
		{
			std::string content( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>( ) );
			file.close( );
			return content;
		}
		else
		{
			eventlogs::get( ).add( crypt_str( "Can't read file: " ) + path, false );
			file.close( );
			return "";
		}
	}
}

sol::state lua;
sol::function original_require;


string DownloadString( string URL );

void c_lua::initialize( )
{
	lua = sol::state( sol::c_call<decltype( &lua_panic ), &lua_panic> );
	lua.open_libraries( sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::base64, sol::lib::bit32, sol::lib::ffi, sol::lib::antiaim_funcs, sol::lib::jit, sol::lib::io, sol::lib::utf8, sol::lib::clipboard, sol::lib::websockets, sol::lib::http, sol::lib::surface, sol::lib::images, sol::lib::easing, sol::lib::vector );

	lua[ crypt_str( "collectgarbage" ) ] = sol::nil;
	lua[ crypt_str( "dofilsse" ) ] = sol::nil;
	lua[ crypt_str( "load" ) ] = sol::nil;
	lua[ crypt_str( "loadfile" ) ] = sol::nil;
	lua[ crypt_str( "pcall" ) ] = sol::nil;
	lua[ crypt_str( "print" ) ] = sol::nil;
	lua[ crypt_str( "xpcall" ) ] = sol::nil;
	lua[ crypt_str( "getmetatable" ) ] = sol::nil;
	lua[ crypt_str( "setmetatable" ) ] = sol::nil;
	lua[ crypt_str( "__nil_callback" ) ] = [ ] ( ) {};

	lua[ "print" ] = [ ] ( std::string s ) { eventlogs::get( ).add( s.c_str( ), true ); };
	lua[ "error" ] = [ ] ( std::string s ) { eventlogs::get( ).luaerror( s.c_str( ), true ); };


	lua[ "require_lib" ] = [ ] ( std::string path ) {

		if ( size_t pos = path.find( "gamesense/" ) != std::string::npos ) {

			std::string module_name = path.substr( pos + 9 );

			auto code = DownloadString( "https://gamesense.fun/forums/luas/" + module_name + ".lua" );

			return lua.require_script( module_name, code );
		}
	};


	lua.new_enum( crypt_str( "key_binds" ),
		crypt_str( "legit_automatic_fire" ), 0,
		crypt_str( "legit_enable" ), 1,
		crypt_str( "double_tap" ), 2,
		crypt_str( "safe_points" ), 3,
		crypt_str( "damage_override" ), 4,
		crypt_str( "hide_shots" ), 12,
		crypt_str( "manual_back" ), 13,
		crypt_str( "manual_left" ), 14,
		crypt_str( "manual_right" ), 15,
		crypt_str( "flip_desync" ), 16,
		crypt_str( "thirdperson" ), 17,
		crypt_str( "automatic_peek" ), 18,
		crypt_str( "edge_jump" ), 19,
		crypt_str( "fakeduck" ), 20,
		crypt_str( "slowwalk" ), 21,
		crypt_str( "body_aim" ), 22
	);

	lua.new_enum( crypt_str( "key_bind_mode" ),
		crypt_str( "hold" ), 0,
		crypt_str( "toggle" ), 1
	);

	lua.new_usertype<entity_t>( crypt_str( "entity" ), // new
		( std::string )crypt_str( "get_prop_int" ), &entity_t::GetPropInt,
		( std::string )crypt_str( "get_prop_float" ), &entity_t::GetPropFloat,
		( std::string )crypt_str( "get_prop_bool" ), &entity_t::GetPropBool,
		( std::string )crypt_str( "get_prop_string" ), &entity_t::GetPropString,
		( std::string )crypt_str( "set_prop_int" ), &entity_t::SetPropInt,
		( std::string )crypt_str( "set_prop_float" ), &entity_t::SetPropFloat,
		( std::string )crypt_str( "set_prop_bool" ), &entity_t::SetPropBool
	);

	lua.new_usertype <Color>( crypt_str( "color" ), sol::constructors <Color( ), Color( int, int, int ), Color( int, int, int, int )>( ),
		( std::string )crypt_str( "r" ), &Color::r,
		( std::string )crypt_str( "g" ), &Color::g,
		( std::string )crypt_str( "b" ), &Color::b,
		( std::string )crypt_str( "a" ), &Color::a
	);

	lua.new_usertype <Vector>( crypt_str( "vector" ), sol::constructors <Vector( ), Vector( float, float, float )>( ),
		( std::string )crypt_str( "x" ), &Vector::x,
		( std::string )crypt_str( "y" ), &Vector::y,
		( std::string )crypt_str( "z" ), &Vector::z,
		( std::string )crypt_str( "length" ), &Vector::Length,
		( std::string )crypt_str( "length_sqr" ), &Vector::LengthSqr,
		( std::string )crypt_str( "length_2d" ), &Vector::Length2D,
		( std::string )crypt_str( "length_2d_sqr" ), &Vector::Length2DSqr,
		( std::string )crypt_str( "is_zero" ), &Vector::IsZero,
		( std::string )crypt_str( "is_valid" ), &Vector::IsValid,
		( std::string )crypt_str( "zero" ), &Vector::Zero,
		( std::string )crypt_str( "dist_to" ), &Vector::DistTo,
		( std::string )crypt_str( "dist_to_sqr" ), &Vector::DistToSqr,
		( std::string )crypt_str( "cross_product" ), &Vector::Cross,
		( std::string )crypt_str( "normalize" ), &Vector::Normalize
	);

	lua.new_usertype<Vector2D>( crypt_str( "Vector2D" ),
		sol::constructors <Vector2D( ), Vector2D( int, int ), Vector( float, float ) >( ),
		( std::string )crypt_str( "x" ), &Vector2D::x,
		( std::string )crypt_str( "y" ), &Vector2D::y,
		( std::string )crypt_str( "Length" ), &Vector2D::Length
	);

	lua.new_usertype <player_info_t>( crypt_str( "player_info" ),
		( std::string )crypt_str( "bot" ), &player_info_t::fakeplayer,
		( std::string )crypt_str( "name" ), &player_info_t::szName,
		( std::string )crypt_str( "steam_id" ), &player_info_t::szSteamID

	);

	lua.new_usertype <IGameEvent>( crypt_str( "game_event" ),
		( std::string )crypt_str( "get_bool" ), &IGameEvent::GetBool,
		( std::string )crypt_str( "get_int" ), &IGameEvent::GetInt,
		( std::string )crypt_str( "get_float" ), &IGameEvent::GetFloat,
		( std::string )crypt_str( "get_string" ), &IGameEvent::GetString,
		( std::string )crypt_str( "set_bool" ), &IGameEvent::SetBool,
		( std::string )crypt_str( "set_int" ), &IGameEvent::SetInt,
		( std::string )crypt_str( "set_float" ), &IGameEvent::SetFloat,
		( std::string )crypt_str( "set_string" ), &IGameEvent::SetString
	);

	lua.new_enum( crypt_str( "hitboxes" ),
		crypt_str( "head" ), HITBOX_HEAD,
		crypt_str( "neck" ), HITBOX_NECK,
		crypt_str( "pelvis" ), HITBOX_PELVIS,
		crypt_str( "stomach" ), HITBOX_STOMACH,
		crypt_str( "lower_chest" ), HITBOX_LOWER_CHEST,
		crypt_str( "chest" ), HITBOX_CHEST,
		crypt_str( "upper_chest" ), HITBOX_UPPER_CHEST,
		crypt_str( "right_thigh" ), HITBOX_RIGHT_THIGH,
		crypt_str( "left_thigh" ), HITBOX_LEFT_THIGH,
		crypt_str( "right_calf" ), HITBOX_RIGHT_CALF,
		crypt_str( "left_calf" ), HITBOX_LEFT_CALF,
		crypt_str( "right_foot" ), HITBOX_RIGHT_FOOT,
		crypt_str( "left_foot" ), HITBOX_LEFT_FOOT,
		crypt_str( "right_hand" ), HITBOX_RIGHT_HAND,
		crypt_str( "left_hand" ), HITBOX_LEFT_HAND,
		crypt_str( "right_upper_arm" ), HITBOX_RIGHT_UPPER_ARM,
		crypt_str( "right_forearm" ), HITBOX_RIGHT_FOREARM,
		crypt_str( "left_upper_arm" ), HITBOX_LEFT_UPPER_ARM,
		crypt_str( "left_forearm" ), HITBOX_LEFT_FOREARM
	);

	lua.new_usertype <player_t>( crypt_str( "player" ), sol::base_classes, sol::bases<entity_t>( ), //new
		( std::string )crypt_str( "get_index" ), &player_t::EntIndex,
		( std::string )crypt_str( "get_dormant" ), &player_t::IsDormant,
		( std::string )crypt_str( "get_team" ), &player_t::m_iTeamNum,
		( std::string )crypt_str( "get_alive" ), &player_t::is_alive,
		( std::string )crypt_str( "get_velocity" ), &player_t::m_vecVelocity,
		( std::string )crypt_str( "get_origin" ), &player_t::GetAbsOrigin,
		( std::string )crypt_str( "get_angles" ), &player_t::m_angEyeAngles,
		( std::string )crypt_str( "get_hitbox_position" ), &player_t::hitbox_position,
		( std::string )crypt_str( "has_helmet" ), &player_t::m_bHasHelmet,
		( std::string )crypt_str( "has_heavy_armor" ), &player_t::m_bHasHeavyArmor,
		( std::string )crypt_str( "is_scoped" ), &player_t::m_bIsScoped,
		( std::string )crypt_str( "get_health" ), &player_t::m_iHealth
	);

	lua.new_usertype <weapon_t>( crypt_str( "weapon" ), sol::base_classes, sol::bases<entity_t>( ),
		( std::string )crypt_str( "is_empty" ), &weapon_t::is_empty,
		( std::string )crypt_str( "can_fire" ), &weapon_t::can_fire,
		( std::string )crypt_str( "is_non_aim" ), &weapon_t::is_non_aim,
		( std::string )crypt_str( "can_double_tap" ), &weapon_t::can_double_tap,
		( std::string )crypt_str( "get_name" ), &weapon_t::get_name,
		( std::string )crypt_str( "get_inaccuracy" ), &weapon_t::get_inaccuracy,
		( std::string )crypt_str( "get_spread" ), &weapon_t::get_spread
	);

	lua.new_enum( crypt_str( "buttons" ),
		crypt_str( "in_attack" ), IN_ATTACK,
		crypt_str( "in_jump" ), IN_JUMP,
		crypt_str( "in_duck" ), IN_DUCK,
		crypt_str( "in_forward" ), IN_FORWARD,
		crypt_str( "in_back" ), IN_BACK,
		crypt_str( "in_use" ), IN_USE,
		crypt_str( "in_cancel" ), IN_CANCEL,
		crypt_str( "in_left" ), IN_LEFT,
		crypt_str( "in_right" ), IN_RIGHT,
		crypt_str( "in_moveleft" ), IN_MOVELEFT,
		crypt_str( "in_moveright" ), IN_MOVERIGHT,
		crypt_str( "in_attack2" ), IN_ATTACK2,
		crypt_str( "in_run" ), IN_RUN,
		crypt_str( "in_reload" ), IN_RELOAD,
		crypt_str( "in_alt1" ), IN_ALT1,
		crypt_str( "in_alt2" ), IN_ALT2,
		crypt_str( "in_score" ), IN_SCORE,
		crypt_str( "in_speed" ), IN_SPEED,
		crypt_str( "in_walk" ), IN_WALK,
		crypt_str( "in_zoom" ), IN_ZOOM,
		crypt_str( "in_weapon1" ), IN_WEAPON1,
		crypt_str( "in_weapon2" ), IN_WEAPON2,
		crypt_str( "in_bullrush" ), IN_BULLRUSH,
		crypt_str( "in_grenade1" ), IN_GRENADE1,
		crypt_str( "in_grenade2" ), IN_GRENADE2,
		crypt_str( "in_lookspin" ), IN_LOOKSPIN
	);
	lua.new_usertype <shot_info>( crypt_str( "shot_info" ), sol::constructors <>( ),
		( std::string )crypt_str( "target_name" ), &shot_info::target_name,
		( std::string )crypt_str( "result" ), &shot_info::result,
		( std::string )crypt_str( "client_hitbox" ), &shot_info::client_hitbox,
		( std::string )crypt_str( "server_hitbox" ), &shot_info::server_hitbox,
		( std::string )crypt_str( "client_damage" ), &shot_info::client_damage,
		( std::string )crypt_str( "server_damage" ), &shot_info::server_damage,
		( std::string )crypt_str( "hitchance" ), &shot_info::hitchance,
		( std::string )crypt_str( "backtrack_ticks" ), &shot_info::backtrack_ticks,
		( std::string )crypt_str( "aim_point" ), &shot_info::aim_point
	);

	auto client = lua.create_table( );
	client[ crypt_str( "set_event_callback" ) ] = ns_client::add_event_callback;
	client[ crypt_str( "set_event_clantag" ) ] = ns_client::add_event_clantag;
	client[ crypt_str( "set_callback" ) ] = ns_client::add_callback;
	client[ crypt_str( "load_script" ) ] = ns_client::load_script;
	client[ crypt_str( "unload_script" ) ] = ns_client::unload_script;
	client[ crypt_str( "log" ) ] = ns_client::log;
	client[ crypt_str( "get_user_name" ) ] = ns_client::get_user_name;
	client[ crypt_str( "is_key_active" ) ] = ns_client::is_key_down;
	client[ crypt_str( "flag" ) ] = ns_client::flag;
	client[ crypt_str( "create_interface" ) ] = ns_utils::create_interface;
	client[ crypt_str( "execute" ) ] = ns_client::execute;
	client[ crypt_str( "unix_time" ) ] = ns_client::get_unix_time;
	client[ crypt_str( "download_string" ) ] = DownloadString;
	client[crypt_str("security_windowsvers")] = ns_client::windowsvers; //new


	auto ui = lua.create_table( );
	ui[ crypt_str( "get" ) ] = ns_menu::get;
	ui[ crypt_str( "is_active" ) ] = ns_menu::is_active;
	ui[ crypt_str( "set_menu_color" ) ] = ns_menu::set_menu_color;
	ui[ crypt_str( "get_menu_color" ) ] = ns_menu::get_menu_color;
	ui[ crypt_str( "get_menu_pos" ) ] = ns_menu::get_menu_pos;
	ui[ crypt_str( "get_menu_size" ) ] = ns_menu::get_menu_size;
	ui[ crypt_str( "get_menu_alpha" ) ] = ns_menu::get_menu_alpha;
	ui[ crypt_str( "next_line" ) ] = ns_menu::next_line;
	ui[ crypt_str( "new_checkbox" ) ] = ns_menu::add_check_box;
	ui[ crypt_str( "new_label" ) ] = ns_menu::new_text;
	ui[ crypt_str( "new_combobox" ) ] = ns_menu::add_combo_box;
	ui[ crypt_str( "disable_tab" ) ] = ns_menu::disable_tab;
	ui[ crypt_str( "text_box" ) ] = ns_menu::add_text_box;
	ui[ crypt_str( "set_visible" ) ] = ns_menu::set_visible;
	ui[ crypt_str( "get_int" ) ] = ns_menu::set_visible;
	ui[ crypt_str( "is_menu_open" ) ] = ns_menu::get_visible;
	ui[ crypt_str( "new_multiselect" ) ] = ns_menu::new_multiselect;
	ui[ crypt_str( "new_listbox" ) ] = ns_menu::add_list_box;
	ui[ crypt_str( "new_hotkey" ) ] = ns_menu::add_key_bind;
	ui[ crypt_str( "set_visibility" ) ] = ns_menu::set_visibility;
	ui[ crypt_str( "new_slider" ) ] = ns_menu::add_slider_int;
	ui[ crypt_str( "new_sliderfl" ) ] = ns_menu::add_slider_float;
	ui[ crypt_str( "new_color_picker" ) ] = ns_menu::add_color_picker;
	ui[ crypt_str( "new_button" ) ] = ns_menu::new_button;
	ui[ crypt_str( "set_bool" ) ] = ns_menu::set_bool;
	ui[ crypt_str( "set_float" ) ] = ns_menu::set_float;
	ui[ crypt_str( "set_color" ) ] = ns_menu::set_color;
	ui[ crypt_str( "reference" ) ] = ns_menu::get;
	ui[ crypt_str( "set" ) ] = sol::overload( ns_menu::set_bool, ns_menu::set_color, ns_menu::set_float, ns_menu::set_multiselect );

	auto globals = lua.create_table( );
	globals[ crypt_str( "get_framerate" ) ] = ns_globals::get_framerate;
	globals[ crypt_str( "get_ping" ) ] = ns_globals::get_ping;
	globals[ crypt_str( "get_server_address" ) ] = ns_globals::get_server_address;
	globals[ crypt_str( "get_time" ) ] = ns_globals::get_time;
	globals[ crypt_str( "get_username" ) ] = ns_globals::get_username;
	globals[ crypt_str( "get_realtime" ) ] = ns_globals::get_realtime;
	globals[ crypt_str( "get_curtime" ) ] = ns_globals::get_curtime;
	globals[ crypt_str( "get_frametime" ) ] = ns_globals::get_frametime;
	globals[ crypt_str( "get_tickcount" ) ] = ns_globals::get_tickcount;
	globals[ crypt_str( "get_framecount" ) ] = ns_globals::get_framecount;
	globals[ crypt_str( "get_intervalpertick" ) ] = ns_globals::get_intervalpertick;
	globals[ crypt_str( "get_maxclients" ) ] = ns_globals::get_maxclients;

	auto engine = lua.create_table( );
	engine[ crypt_str( "get_screen_size" ) ] = ns_engine::get_screen_size;
	engine[ crypt_str( "get_level_name" ) ] = ns_engine::get_level_name;
	engine[ crypt_str( "get_level_name_short" ) ] = ns_engine::get_level_name_short;
	engine[ crypt_str( "get_local_player_index" ) ] = ns_engine::get_local_player_index;
	engine[ crypt_str( "get_map_group_name" ) ] = ns_engine::get_map_group_name;
	engine[ crypt_str( "get_player_for_user_id" ) ] = ns_engine::get_player_for_user_id;
	engine[ crypt_str( "get_player_info" ) ] = ns_engine::get_player_info;
	engine[ crypt_str( "get_view_angles" ) ] = ns_engine::get_view_angles;
	engine[ crypt_str( "is_connected" ) ] = ns_engine::is_connected;
	engine[ crypt_str( "is_hltv" ) ] = ns_engine::is_hltv;
	engine[ crypt_str( "is_in_game" ) ] = ns_engine::is_in_game;
	engine[ crypt_str( "is_paused" ) ] = ns_engine::is_paused;
	engine[ crypt_str( "get_cursor_pos" ) ] = ns_render::get_cursor_pos;
	engine[ crypt_str( "is_playing_demo" ) ] = ns_engine::is_playing_demo;
	engine[ crypt_str( "is_recording_demo" ) ] = ns_engine::is_recording_demo;
	engine[ crypt_str( "is_taking_screenshot" ) ] = ns_engine::is_taking_screenshot;
	engine[ crypt_str( "set_view_angles" ) ] = ns_engine::set_view_angles;

	auto renderer = lua.create_table( );
	renderer[ crypt_str( "indicator" ) ] = ns_render::add_indicator;//new
	renderer[ crypt_str( "world_to_screen" ) ] = ns_render::world_to_screen;
	renderer[ crypt_str( "get_text_width" ) ] = ns_render::get_text_size;
	renderer[ crypt_str( "create_font" ) ] = ns_render::create_font;
	renderer[ crypt_str( "text" ) ] = ns_render::draw_text;
	renderer[ crypt_str( "text_centered" ) ] = ns_render::draw_text_centered;
	renderer[ crypt_str( "text_small" ) ] = ns_render::draw_text_small;
	renderer[ crypt_str( "line" ) ] = ns_render::draw_line;
	renderer[ crypt_str( "add_side_indicator" ) ] = ns_render::add_side_indicator;
	renderer[ crypt_str( "rectangle" ) ] = ns_render::draw_rect;
	renderer[ crypt_str( "load_texture" ) ] = ns_render::load_texture;
	renderer[ crypt_str( "rectangle_rounded" ) ] = ns_render::draw_rounded_rect;
	renderer[ crypt_str( "rectangle_filled" ) ] = ns_render::draw_rect_filled;
	renderer[ crypt_str( "rectangle_filled_gradient" ) ] = ns_render::draw_rect_filled_gradient;
	renderer[ crypt_str( "circle" ) ] = ns_render::draw_circle;
	renderer[ crypt_str( "circle_filled" ) ] = ns_render::draw_circle_filled;
	renderer[ crypt_str( "triangle" ) ] = ns_render::draw_triangle;
	renderer[ crypt_str( "logmesg" ) ] = ns_render::draw_logmsg;

	//renderer[crypt_str("create_texture")] = ns_render::create_texture;
	renderer[ crypt_str( "texture" ) ] = ns_render::render_texture;

	auto console = lua.create_table( );
	console[ crypt_str( "get_int" ) ] = ns_console::get_int;
	console[ crypt_str( "get_float" ) ] = ns_console::get_float;
	console[ crypt_str( "get_string" ) ] = ns_console::get_string;
	console[ crypt_str( "set_int" ) ] = ns_console::set_int;
	console[ crypt_str( "set_float" ) ] = ns_console::set_float;
	console[ crypt_str( "set_string" ) ] = ns_console::set_string;

	auto events = lua.create_table( ); //-V688
	events[ crypt_str( "register_event" ) ] = ns_events::register_event;

	auto entitylist = lua.create_table( );
	entitylist[ crypt_str( "get_local_player" ) ] = ns_entitylist::get_local_player;
	entitylist[ crypt_str( "get_player_by_index" ) ] = ns_entitylist::get_player_by_index;
	entitylist[ crypt_str( "get_weapon_by_player" ) ] = ns_entitylist::get_weapon_by_player;
	entitylist[ crypt_str( "get_local_player_index" ) ] = ns_entitylist::get_local_player_index;

	auto utils = lua.create_table( );//new
	utils[ crypt_str( "find_signature" ) ] = ns_utils::find_signature;//new
	utils[ crypt_str( "create_interface" ) ] = ns_utils::create_interface;//new

	auto cmd = lua.create_table( );
	cmd[ crypt_str( "get_send_packet" ) ] = ns_cmd::get_send_packet;
	cmd[ crypt_str( "set_send_packet" ) ] = ns_cmd::set_send_packet;
	cmd[ crypt_str( "get_choke" ) ] = ns_cmd::get_choke;
	cmd[ crypt_str( "get_button_state" ) ] = ns_cmd::get_button_state;
	cmd[ crypt_str( "set_button_state" ) ] = ns_cmd::set_button_state;


	auto exploits = lua.create_table( );
	exploits[ crypt_str( "override_shift" ) ] = ns_exploits::set_max_shift_value;
	exploits[ crypt_str( "override_tolerance" ) ] = ns_exploits::set_rc_time;

	auto external = lua.create_table( );//new
	external[ crypt_str( "get" ) ] = ns_http::get;//new
	external[ crypt_str( "post" ) ] = ns_http::post;//new

	auto db = lua.create_table( );//new
	db[ crypt_str( "append" ) ] = ns_file::append; //new
	db[ crypt_str( "write" ) ] = ns_file::write;//new
	db[ crypt_str( "read" ) ] = ns_file::read;//new

	lua[ crypt_str( "client" ) ] = client;
	lua[ crypt_str( "ui" ) ] = ui;
	lua[ crypt_str( "globals" ) ] = globals;
	lua[ crypt_str( "engine" ) ] = engine;
	lua[ crypt_str( "renderer" ) ] = renderer;
	lua[ crypt_str( "console" ) ] = console;
	lua[ crypt_str( "events" ) ] = events;
	lua[ crypt_str( "entitylist" ) ] = entitylist;
	lua[ crypt_str( "cmd" ) ] = cmd;
	lua[ crypt_str( "utils" ) ] = utils;
	lua[ crypt_str( "external" ) ] = external;//new
	lua[ crypt_str( "db" ) ] = db;//new
	lua[ crypt_str( "exploits" ) ] = exploits;//new
	//refresh_scripts_startup();

	char i_buffer[ 65536 ] = { 0 };

	//int i_read = GetPrivateProfileSectionA("autoload", i_buffer, 65536, "C:\\gs\\luas\\menu.ini");

	//if ((0 < i_read) && ((65536 - 2) > i_read)) {
	//	const char* pSubstr = i_buffer;

	//	while ('\0' != *pSubstr) {
	//		size_t substrLen = strlen(pSubstr);

	//		const char* pos = strchr(pSubstr, '=');
	//		if (NULL != pos) {
	//			char name[256] = "";
	//			char value[256] = "";

	//			strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
	//			strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

	//			if (atoi(value) == 1) {
	//				this->load_script(this->get_script_id(name));
	//			}


	//		}

	//		pSubstr += (substrLen + 1);
	//	}

	//}

}

#include <comdef.h>
#include <string> 
#include <Wininet.h>

string replaceAll( string subject, const string& search,
	const string& replace ) {
	size_t pos = 0;
	while ( ( pos = subject.find( search, pos ) ) != string::npos ) {
		subject.replace( pos, search.length( ), replace );
		pos += replace.length( );
	}
	return subject;
}

string DownloadString( string URL ) {
	HINTERNET interwebs = InternetOpenA( "Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL );
	HINTERNET urlFile;
	string rtn;
	if ( interwebs ) {
		urlFile = InternetOpenUrlA( interwebs, URL.c_str( ), NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, NULL );
		if ( urlFile ) {
			char buffer[ 2000 ];
			DWORD bytesRead;
			do {
				InternetReadFile( urlFile, buffer, 2000, &bytesRead );
				rtn.append( buffer, bytesRead );
				memset( buffer, 0, 2000 );
			} while ( bytesRead );
			InternetCloseHandle( interwebs );
			InternetCloseHandle( urlFile );
			string p = replaceAll( rtn, "|n", "\r\n" );
			return p;
		}
	}
	InternetCloseHandle( interwebs );
	string p = replaceAll( rtn, "|n", "\r\n" );
	return p;
}

std::string get_username( ) {
	DWORD BufferSize = 20000;
	char szUsername[ 20000 ] = {};
	std::string username; long pro; HKEY hKeyp;
	RegGetValue( HKEY_CURRENT_USER, crypt_str( "SOFTWARE\\gsfun" ), crypt_str( "login" ), RRF_RT_ANY, NULL, ( PVOID )&szUsername, &BufferSize );
	username = szUsername;
	return username;
}

bool c_lua::is_cloud_lua( std::string name ) {
	if ( name.front( ) == '*' ) {
		return true;
	}

	return false;
}


int c_lua::get_script_id( std::string name ) {
	if ( name.front( ) == '*' ) {
		name.erase( remove( name.begin( ), name.end( ), '*' ), name.end( ) );
	}

	for ( int i = 0; i < this->scripts.size( ); i++ ) {
		if ( this->scripts.at( i ) == name )
			return i;
	}
	return -1;
}

int c_lua::get_script_id_by_path( std::string path ) {
	for ( int i = 0; i < this->pathes.size( ); i++ ) {
		if ( this->pathes.at( i ).string( ) == path )
			return i;
	}

	return -1;
}

void c_lua::refresh_scripts( )
{
	auto oldLoaded = loaded;
	auto oldStartup = startup;
	auto oldScripts = scripts;

	startup.clear( );
	loaded.clear( );
	pathes.clear( );
	scripts.clear( );
	scripts_cloud.clear( );
	scripts_names.clear( );
	ns_console::convars.clear( );

	std::vector<std::filesystem::path> pathes_to_scan = { "C:\\gs\\luas\\cloud","C:\\gs\\luas" };
	CreateDirectory( "C:\\gs\\luas\\", NULL );
	CreateDirectory( "C:\\gs\\luas\\cloud\\", NULL );
	for ( int l = 0; l < 2; l++ ) {

		for ( auto& entry : std::filesystem::directory_iterator( pathes_to_scan.at( l ) ) )
		{
			if ( entry.path( ).extension( ) == crypt_str( ".lua" ) || entry.path( ).extension( ) == crypt_str( ".luac" ) )
			{
				auto path = entry.path( );
				auto filename = path.filename( ).string( );

				auto didPut = false;



				for ( auto i = 0; i < oldScripts.size( ); i++ )
				{
					if ( filename == oldScripts.at( i ) ) //-V106
					{
						loaded.push_back( oldLoaded.at( i ) ); //-V106
						startup.push_back( oldLoaded.at( i ) );
						didPut = true;
					}
				}

				if ( !didPut )
					loaded.push_back( false );

				pathes.push_back( path );
				scripts.push_back( filename );

				if ( l == 0 ) {
					scripts_cloud.push_back( "*" + filename );
					scripts_names.push_back( "*" + filename );
				}
				else {
					scripts_names.push_back( filename );
				}
			}
		}
	}

}

void c_lua::refresh_scripts_startup( )
{
	auto oldLoaded = loaded;
	auto oldStartup = startup;
	auto oldScripts = scripts;

	startup.clear( );
	loaded.clear( );
	pathes.clear( );
	scripts.clear( );
	scripts_cloud.clear( );
	scripts_names.clear( );
	ns_console::convars.clear( );

	std::vector<std::filesystem::path> pathes_to_scan = { "C:\\gs\\luas\\cloud","C:\\gs\\luas" };
	CreateDirectory( "C:\\gs\\luas\\", NULL );
	CreateDirectory( "C:\\gs\\luas\\cloud\\", NULL );
	for ( int l = 0; l < 2; l++ ) {

		for ( auto& entry : std::filesystem::directory_iterator( pathes_to_scan.at( l ) ) )
		{
			if ( entry.path( ).extension( ) == crypt_str( ".lua" ) || entry.path( ).extension( ) == crypt_str( ".luac" ) )
			{
				auto path = entry.path( );
				auto filename = path.filename( ).string( );

				auto didPut = false;



				for ( auto i = 0; i < oldScripts.size( ); i++ )
				{
					if ( filename == oldScripts.at( i ) ) //-V106
					{
						loaded.push_back( oldLoaded.at( i ) ); //-V106
						startup.push_back( oldLoaded.at( i ) );
						didPut = true;
					}
				}

				if ( !didPut )
					loaded.push_back( false );

				pathes.push_back( path );
				scripts.push_back( filename );

				if ( l == 0 ) {
					scripts_cloud.push_back( "*" + filename );
					scripts_names.push_back( "*" + filename );
					this->load_script( this->get_script_id( "*" + filename ) );
				}
				else {
					scripts_names.push_back( filename );
				}
			}
		}
	}

}

void c_lua::download_cloud_scripts( ) {
	std::vector<std::string> urls;
	std::string beta;
	std::string beta222;

	DWORD BufferSize = 20000;
	char szUsername[ 20000 ] = {};


	RegGetValue( HKEY_CURRENT_USER, crypt_str( "SOFTWARE\\gsfun" ), crypt_str( "login" ), RRF_RT_ANY, NULL, ( PVOID )&szUsername, &BufferSize );
	std::string result = szUsername;

	std::string username = DownloadString( crypt_str( "https://gamesense.ltd/forums/cloud/get_id?username=" ) + result );

	crypt_str( beta222 ) = crypt_str( DownloadString( crypt_str( "https://gamesense.ltd/forums/cloud/amm-get.php?lua_id=" ) + username ) );

	if ( crypt_str( beta222 ) == "0" )
		return;
	std::stringstream geek( crypt_str( beta222 ) );

	for ( const auto& entry : std::filesystem::directory_iterator( crypt_str( "C:\\gs\\luas\\cloud" ) ) )
		std::filesystem::remove_all( entry.path( ) );

	std::vector<std::string> presets;
	int i = 1, c;
	geek >> c;
	for ( int i = 1; i < ( c + 1 ); i++ ) {
		beta = crypt_str( DownloadString( crypt_str( "https://gamesense.fun/forums/cloud/scripts-get.php?lua_id=" ) + crypt_str( username ) + "&i=" + std::to_string( i ) ) );
		presets.push_back( crypt_str( beta ) );
	}

	for ( auto& s : presets ) {
		//Utilities->Game_Msg(s);
		std::string beta2;
		std::string beta3;
		beta2 = crypt_str( DownloadString( crypt_str( "https://gamesense.fun/forums/cloud/cloud-get.php?lua_id=" ) + crypt_str( s ) ) ); //using my uid for testing purposes for now
		beta3 = crypt_str( DownloadString( crypt_str( "https://gamesense.fun/forums/cloud/cloud2.php?lua_id=" ) + crypt_str( s ) ) ); //using my uid for testing purposes for now

		string savepath = crypt_str( "C:\\gs\\luas\\cloud" + beta2 );
		HRESULT hr = crypt_str( URLDownloadToFile( NULL, crypt_str( beta3.c_str( ) ), crypt_str( savepath.c_str( ) ), 0, NULL ) );

	}
	string cement = crypt_str( "Your subscribed scripts have been added" );
	eventlogs::get( ).add( crypt_str( cement ), true );
}

void c_lua::load_script( int id )
{
	if ( id == -1 )
		return;

	if ( loaded.at( id ) ) //-V106
		return;

	auto path = get_script_path( id );

	if ( path == crypt_str( "" ) )
		return;

	auto error_load = false;
	loaded.at( id ) = true;
	lua.script_file( path,
		[ &error_load ] ( lua_State*, sol::protected_function_result result )
		{
			if ( !result.valid( ) )
			{
				sol::error error = result;
				auto log = crypt_str( "Lua error: " ) + ( std::string )error.what( );
				m_surface( )->PlaySound_( crypt_str( "UI/weapon_cant_buy.wav" ) );
				eventlogs::get( ).luaerror( log, false );
				error_load = true;

			}

			return result;
		}
	);

	if ( error_load | loaded.at( id ) == false )
	{
		loaded.at( id ) = false;
		return;
	}


	//-V106
	g_ctx.globals.loaded_script = true;
}

void c_lua::unload_script( int id ) {
	if ( id == -1 )
		return;

	if ( !loaded.at( id ) )
		return;

	if ( g_ctx.globals.loaded_script )
		for ( auto current : c_lua::get( ).hooks.getHooks( crypt_str( "on_unload" ) ) )
			current.func( );

	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> updated_items;
	for ( auto i : this->menu_items ) {
		for ( auto k : i.second ) {
			std::vector<MenuItem_t> updated_vec;

			for ( auto m : k.second )
				if ( m.script != id )
					updated_vec.push_back( m );

			updated_items[ k.first ][ i.first ] = updated_vec;
		}
	}
	this->menu_items = updated_items;

	if ( c_lua::get( ).events.find( id ) != c_lua::get( ).events.end( ) )
		c_lua::get( ).events.at( id ).clear( );

	g_ctx.globals.loaded_script = false;

	hooks.unregisterHooks( id );
	loaded.at( id ) = false;


}

void c_lua::reload_all_scripts( )
{
	for ( auto current : scripts )
	{
		if ( !loaded.at( get_script_id( current ) ) ) //-V106
			continue;

		unload_script( get_script_id( current ) );
		load_script( get_script_id( current ) );
	}
}

void c_lua::unload_all_scripts( )
{
	for ( auto s : scripts )
		unload_script( get_script_id( s ) );
}

std::string c_lua::get_script_path( std::string name ) {
	return this->get_script_path( this->get_script_id( name ) );
}

std::string c_lua::get_script_path( int id ) {
	if ( id == -1 )
		return  "";

	return this->pathes.at( id ).string( );
}
