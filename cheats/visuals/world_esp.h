#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class weapon_t;
class Box;

class worldesp : public singleton <worldesp> 
{
public:
	player_t* player = nullptr;

	void paint_traverse();
	void skybox_changer();
	void fog_changer();
	void world_modulation(entity_t* entity);
	void molotov_timer(entity_t* entity);
	//void molotov_timer(entity_t* entity, int id);
	void smoke_timer(entity_t* entity);
	void grenade_projectiles(entity_t* entity);
	void bomb_timer(entity_t* entity);
	Vector											m_origin{}, m_velocity{};
	IClientEntity* m_last_hit_entity{};
	Collision_Group_t								m_collision_group{};
	float											m_detonate_time{}, m_expire_time{};
	int												m_index{}, m_tick{}, m_next_think_tick{};
	void dropped_weapons(entity_t* entity);

	bool changed = false;
	std::string backup_skybox = "";
};