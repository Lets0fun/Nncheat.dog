// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "nightmode.h"

std::vector <MaterialBackup> materials;

void nightmode::clear_stored_materials() 
{
	materials.clear();
}

void nightmode::modulate(MaterialHandle_t i, IMaterial* material, bool backup = false) 
{
	auto name = material->GetTextureGroupName();

	Color wrldcol
	{
		c_config::get()->c["adjcol"][0],
		c_config::get()->c["adjcol"][1],
		c_config::get()->c["adjcol"][2],
		c_config::get()->c["adjcol"][3],
		//(int)( c_config::get( )->i[ "transwalls" ] * 2.55 )
	};

	Color propcol
	{
		c_config::get()->c["propecoll"][0],
		c_config::get()->c["propecoll"][1],
		c_config::get()->c["propecoll"][2],
		//( int ) ( c_config::get( )->i[ "transprops" ] * 2.55 )
		c_config::get()->c["propecoll"][3]
	};

	if (strstr(name, crypt_str("World")))
	{
		if (backup) 
			materials.emplace_back(MaterialBackup(i, material));

		material->AlphaModulate((float)wrldcol.a() / 255.0f);
		material->ColorModulate((float)wrldcol.r() / 255.0f, (float)wrldcol.g() / 255.0f, (float)wrldcol.b() / 255.0f);
	}
	else if (strstr(name, crypt_str("StaticProp")))
	{
		if (backup) 
			materials.emplace_back(MaterialBackup(i, material));

		material->AlphaModulate((float)propcol.a() / 255.0f);
		material->ColorModulate((float)propcol.r() / 255.0f, (float)propcol.g() / 255.0f, (float)propcol.b() / 255.0f);
	}
}

void nightmode::apply()
{
	if (!materials.empty())
	{
		for (auto i = 0; i < (int)materials.size(); i++) //-V202
			modulate(materials[i].handle, materials[i].material);

		return;
	}

	materials.clear();
	auto materialsystem = m_materialsystem();

	for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
	{
		auto material = materialsystem->GetMaterial(i);

		if (!material)
			continue;

		if (material->IsErrorMaterial())
			continue;

		modulate(i, material, true);
	}
}

void nightmode::remove() 
{
	for (auto i = 0; i < materials.size(); i++)
	{
		if (!materials[i].material)
			continue;

		if (materials[i].material->IsErrorMaterial())
			continue;

		materials[i].restore();
		materials[i].material->Refresh();
	}

	materials.clear();
}