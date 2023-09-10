#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

struct Box;

class playeresp : public singleton <playeresp>
{
public:
	int type = ENEMY;
	float esp_alpha_fade[65];
	int health[65];
	int health2[65];
	HPInfo hp_info[65];

	IDirect3DDevice9* esp_device;

	void paint_traverse();
	void centerinds();
	void draw_box(player_t* m_entity, const Box& box);
	void draw_health(player_t* m_entity, const Box& box, const HPInfo& hpbox);
	void draw_skeleton(player_t* e, Color color, matrix3x4_t matrix[MAXSTUDIOBONES]);
	bool draw_ammobar(player_t* m_entity, const Box& box);
	void draw_name(player_t* m_entity, const Box& box);
	void draw_weapon(player_t* m_entity, const Box& box, bool space);
	void draw_flags(player_t* e, const Box& box);
	void draw_lines(player_t* e);
	void draw_multi_points(player_t* e);

	struct SideInfo {
		std::string text = "";
		D3DCOLOR color = D3DCOLOR_RGBA(255, 255, 255, 200);

		SideInfo(std::string pText, D3DCOLOR pColor) {
			text = pText;
			color = pColor;
		};
	};

};