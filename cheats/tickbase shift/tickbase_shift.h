#pragma once

#include "..\..\includes.hpp"

class exploit : public singleton <exploit>
{
private:

public:

	void BreakLagCompensation(CUserCmd* m_pcmd);
	//bool createmove(CUserCmd* m_pcmd);
	bool CanDoubleTap(bool check_charge = false);
	//void double_tap_deffensive(CUserCmd* m_pcmd);
	bool defensive(CUserCmd* m_pcmd);
	void DoubleTap(CUserCmd* m_pcmd);
	void HideShots(CUserCmd* m_pcmd);

	bool recharging_double_tap = false;

	bool double_tap_enabled = false;
	bool double_tap_key = false;

	bool hide_shots_enabled = false;
	bool hide_shots_key = false;

	bool uncharged_this_tick = false;

	int lastdoubletaptime = 0;
};