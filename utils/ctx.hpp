#pragma once
#include "..\ImGui\imgui.h"
#include "..\includes.hpp"
#include "..\utils\crypt_str.h"

class player_t;
class weapon_t;
class CUserCmd;

struct shot_info
{
	bool should_log = false;

	std::string target_name = crypt_str("None");
	std::string result = crypt_str("None");

	std::string client_hitbox = crypt_str("None");
	std::string server_hitbox = crypt_str("None");

	int client_damage = 0;
	int server_damage = 0;

	int hitchance = 0;
	int backtrack_ticks = 0;

	Vector aim_point = ZERO;
};

struct History_data
{
	int type_normal = 0;
	bool type_low_delta = false;

	void reset()
	{
		type_normal = 0;
		type_low_delta = false;
	}
};

struct aim_shot
{
	bool start = false;
	bool end = false;
	bool impacts = false;
	bool latency = false;
	bool hurt_player = false;
	bool impact_hit_player = false;
	bool occlusion = false;
	int client_hitbox_int = 0;

	int last_target = -1;
	int side = -1;
	int fire_tick = INT_MAX;
	int event_fire_tick = INT_MAX;

	shot_info shot_info;
};

struct command
{
	bool is_outgoing = false;
	bool is_used = false;
	int previous_command_number = 0;
	int command_number = 0;
};

struct correction_data
{
	int command_number = 0;
	int tickcount = 0;
	int choked_commands = 0;
};

class ctx_t  //-V730
{
	CUserCmd* m_pcmd = nullptr;
public:
	struct Globals  //-V730
	{
		bool loaded_script = false;
		bool focused_on_input = false;
		bool double_tap_fire = false;
		bool double_tap_aim = false;
		bool double_tap_aim_check = false;
		bool fired_shot = false;
		bool force_send_packet = false;
		bool exploits = false;
		bool scoped = false;
		bool grenade = false;
		bool autowalling = false;
		bool setuping_bones = false;
		bool updating_animation = false;
		bool aimbot_working = false;
		bool revolver_working = false;
		bool slowwalking = false;
		bool change_materials = false;
		bool drawing_ragdoll = false;
		bool in_thirdperson = true;
		bool fakeducking = false;
		bool standing = false;
		bool should_choke_packet = false;
		bool should_send_packet = false;
		bool bomb_timer_enable = false;
		bool backup_model = false;
		bool reset_net_channel = false;
		bool in_createmove = false;
		bool should_remove_smoke = false;
		bool should_update_beam_index = false;
		bool should_clear_death_notices = false;
		bool should_update_playerresource = false;
		bool should_update_gamerules = false;
		bool should_check_skybox = false;
		bool should_update_radar = false;
		bool updating_skins = false;
		bool should_update_weather = false;
		bool m_bIsLocalPeek;
		int shot_command = 0;
		bool m_bInDiffensiveDt;
		int tocharge;
		int tochargeamount = 0;
		int dt_shots = 0;
		struct new_dt_t
		{
			int next_tickbase_shift = 0;
			int tickbase_shift = 0;
			int charge_ticks = 0;
			bool recharging = false;
			bool shifting = false;
		}new_dt;

		int framerate = 0;
		int shift_command = 0;
		int shift_amount = 0;
		int ping = 0;
		int ticks_allowed = 0;
		IDirect3DDevice9* device;
		int ticks_choke = 0;
		int next_tickbase_shift = 0;
		int tickbase_shift = 0;
		int shift_timer = 0;
		int tickbase_shifted_command = 0;
		int current_tickbase_shift = 0;
		int original_tickbase = 0;
		int fixed_tickbase = 0;
		int shifted_ticks = 0;
		int backup_tickbase = 0;

		bool should_recharge = false;
		bool trigger_teleport = false;
		int teleport_amount = 0;
		bool isshifting = true;
		int shift_ticks = 1;
		bool startcharge = true;
		struct
		{
			int m_PeekTick = -1;
			int32_t m_LastShift = 0;
			bool m_bIsPeeking = false;
			player_t* m_Player = NULL;
			bool started_peeking_Defensive;
		} m_Peek;

		int current_weapon = 0;
		int last_aimbot_shot = 0;
		int bomb_carrier = 0;
		int kills = 0;
		int should_buy = 0;
		int fired_shots[65];
		int missed_shots[65];
		int missed_shots_spread[65];
		int tickrate = 0;


		int out_sequence_nr = 0;
		bool shifting;
		int m_shifted_command;
		int last_cmd_delta = 0;
		int current_tickcount = 0;

		float next_lby_update = 0.0f;
		float last_lby_move = 0.0f;
		float weap_inaccuracy = 0.0f;
		float inaccuracy = 0.0f;
		float spread = 0.0f;
		float last_velocity_modifier = 0.0f;
		float original_forwardmove = 0.0f;
		float original_sidemove = 0.0f;

		bool override_velmod = false;
		Vector original;
		int indicator_pos = 0;

		std::string time = crypt_str("unknown");

		Vector eye_pos = ZERO;
		Vector start_position = ZERO;
		Vector dormant_origin[65];

		matrix3x4_t prediction_matrix[MAXSTUDIOBONES];
		matrix3x4_t fake_matrix[MAXSTUDIOBONES];

		IClientNetworkable* m_networkable = nullptr;
		weapon_t* weapon = nullptr;
		std::vector <int> choked_number;
		std::deque <command> commands;
		std::deque <correction_data> data;
		std::vector <std::string> events;

		History_data history_data[65];

		ImVec4 menu_color;
	} globals;

	struct gui_helpers
	{
		bool open_pop;
		float pop_anim;
	}gui;

	struct r_info
	{
		float m_possible_delta = 0.0f;
		int m_possible_side = 0;
		bool m_abusing_low_rotation = false;
		bool m_abusing_left_rotation = false;
		bool m_abusing_right_rotation = false;
		bool m_validated_move = false;
	} r_info;

	std::vector <std::string> signatures;
	std::vector <int> indexes;

#if RELEASE
	std::string username;
#else
	std::string username = crypt_str("0xF");
#endif

	std::string last_font_name;

	std::vector <aim_shot> shots;

	bool available();

	bool send_packet = false;

	bool canBreakLC, canDrawLC;

	void set_command(CUserCmd* cmd)
	{
		m_pcmd = cmd;
	}

	player_t* local(player_t* e = nullptr, bool initialization = false);
	CUserCmd* get_command();
};

extern ctx_t g_ctx;