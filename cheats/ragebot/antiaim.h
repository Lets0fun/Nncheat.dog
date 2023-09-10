#pragma once

#include "..\autowall\autowall.h"
#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class antiaim : public singleton <antiaim>
{
public:
	void create_move(CUserCmd* m_pcmd);
	//void jitter_move(CUserCmd* m_pcmd);
	//player_t* GetNearestTarget(bool check = false);
	float get_pitch(CUserCmd* m_pcmd);
	float skitter_jitter(CUserCmd* m_pcmd);
	float get_roll(CUserCmd* m_pcmd);
	float get_yaw(CUserCmd* m_pcmd);
	bool ShouldBreakLowerBody(CUserCmd* m_pcmd, int lby_type);
	bool condition(CUserCmd* m_pcmd, bool dynamic_check = true);

	float at_targets();
	bool automatic_direction();
	void freestanding(CUserCmd* m_pcmd);
	void koppel_fs(CUserCmd* m_pcmd);

	float m_flAccelerationSpeed = 95.0f;
	float local_pitch = 0;
	bool m_sidemove;
	int type = 0;
	int manual_side = -1;
	int final_manual_side = -1;
	bool flip = false;
	bool freeze_check = false;
	bool breaking_lby = false;
	float desync_angle = 0.0f;

	struct angle_data {
		float angle;
		float thickness;
		angle_data(const float angle, const float thickness) : angle(angle), thickness(thickness) {}
	};
};

enum
{
	SIDE_NONE = -1,
	SIDE_BACK,
	SIDE_LEFT,
	SIDE_RIGHT,
	SIDE_FORWARD
};