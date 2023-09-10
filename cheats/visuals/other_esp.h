#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

struct Indicator_t
{
	std::string str;
	D3DCOLOR color;
};

class otheresp : public singleton< otheresp >
{
public:
	void penetration_reticle();
	void indicators();
	void create_fonts();
	void draw_indicators();
	void hitmarker_paint();
	void spread_crosshair(LPDIRECT3DDEVICE9 device);
	void automatic_peek_indicator();

	struct Hitmarker
	{
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::White;
		Vector point = ZERO;
	} hitmarker;

	struct C_TrailSegment
	{
		Vector position = Vector(0, 0, 0);
		float expiration = -1;

		std::vector< C_TrailSegment > trail_segments;
		Vector last_origin;
	}trail_data;

	struct Damage_marker
	{
		Vector position = ZERO;
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::Red;
		int damage = -1;
		int hitgroup = -1;

		void reset()
		{
			position.Zero();
			hurt_time = FLT_MIN;
			hurt_color = Color::White;
			damage = -1;
			hitgroup = -1;
		}
	} damage_marker[65];

	vgui::HFont IndFont;
	vgui::HFont IndFont125;
	vgui::HFont IndFont150;
	vgui::HFont IndFont175;
	vgui::HFont IndFont200;
	vgui::HFont IndShadow;
	vgui::HFont NameFontESP;
	vgui::HFont SmallFontESP;
	std::vector<Indicator_t> m_indicators;
};