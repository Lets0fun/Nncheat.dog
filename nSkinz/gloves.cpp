#include "gloves.hpp"

bool SkinChanger::Gloves::Enable = true;
int SkinChanger::Gloves::TGlove = 0;
int SkinChanger::Gloves::CTGlove = 0;
std::vector<std::string> SkinChanger::Gloves::GloveOptions = { "Bloodhound", "Sporty", "Slick", "Leather", "Motorcycle", "Specialist" };
std::vector<int> GloveIndices = { 5027, 5030, 5031, 5032, 5033, 5034 };
std::vector<std::vector<std::string>> SkinChanger::Gloves::SkinsByGlove = { { "Black Silver", "Snakeskin Brass", "Metallic", "Guerrilla" }, { "Light Blue", "Military", "Purple", "Green" }, { "Black", "Military", "Red", "Snakeskin Yellow" }, { "Red Slaughter", "Orange Camo" }, { "Basic Black", "Mint Triangle", "Mono Boom", "Triangle Blue" }, { "Green Camo", "Red Kimono", "Emerald Web", "Orange White" } };
std::vector<std::vector<size_t>> PaintByGlove = { { 10006, 10007, 10008, 10039 }, {10018, 10019, 10037, 10038 }, { 10013, 10015, 10016, 10040 }, { 10021, 10036 }, { 10024, 10026, 10027, 10028 }, { 10030, 10033, 10034, 10035 } };
int SkinChanger::Gloves::TSkin = 0;
std::vector<std::string> SkinChanger::Gloves::TSkinOptions = SkinChanger::Gloves::SkinsByGlove[ 0 ];
int SkinChanger::Gloves::CTSkin = 0;
std::vector<std::string> SkinChanger::Gloves::CTSkinOptions = SkinChanger::Gloves::SkinsByGlove[ 0 ];

TeamID currentTeam = TeamID::TEAM_UNASSIGNED;

namespace GloveChanger
{
	bool NeedsUpdate = false;

	WearableObject_t::WearableObject_t( int index, int paint, int seed, int stattrak, int quality, float wear, char* fileName, bool updateStatus )
	{
		Index = index;
		Paintkit = paint;
		Seed = seed;
		Stattrak = stattrak;
		EntityQuality = quality;
		Wear = wear;
		ModelFileName = fileName;
		UpdateStatus = updateStatus;
	}

	GloveSkin_t GetGloveSkin( bool TSide )
	{
		size_t glove = TSide ? SkinChanger::Gloves::TGlove : SkinChanger::Gloves::CTGlove;
		size_t skin = TSide ? SkinChanger::Gloves::TSkin : SkinChanger::Gloves::CTSkin;

		return { ( ItemDefinitionIndex )GloveIndices[ glove ], PaintByGlove[ glove ][ skin ] };
	}

	void FrameStageNotify( )
	{

		if ( !m_engine( )->IsInGame( ) )
			return;

		if ( !SkinChanger::Gloves::Enable )
			return;

		if ( !g_ctx.local( ) )
			return;

		player_info_t localplayer_info;
		if ( !m_engine( )->GetPlayerInfo( m_engine( )->GetLocalPlayer( ), &localplayer_info ) )
			return;

		if ( !g_ctx.local( )->is_alive( ) )
		{

			attributableitem_t* glove = ( attributableitem_t* )m_entitylist( )->GetClientEntity( g_ctx.local( )->m_hMyWearables( )[ 0 ] & 0xFFF );
			if ( !glove )
				return;

			glove->GetClientNetworkable( )->SetDestroyedOnRecreateEntities( );
			glove->GetClientNetworkable( )->Release( );

			return;
		}

		if ( !m_entitylist( )->GetClientEntity( g_ctx.local( )->m_hMyWearables( )[ 0 ] & 0xFFF ) )
		{
			for ( ClientClass* pClass = m_client( )->GetAllClasses( ); pClass; pClass = pClass->m_pNext )
			{
				if ( pClass->m_ClassID != CEconWearable )
					continue;

				int entry = ( m_entitylist( )->GetHighestEntityIndex( ) + 1 );
				int serial = util::RandomInt( 0x0, 0xFFF );

				pClass->m_pCreateFn( entry, serial );
				g_ctx.local( )->m_hMyWearables( )[ 0 ] = entry | ( serial << 16 );

				//SkinChanger::forceFullUpdate = true;
				NeedsUpdate = true;

				break;
			}
		}

		attributableitem_t* glove = ( attributableitem_t* )m_entitylist( )->GetClientEntity( g_ctx.local( )->m_hMyWearables( )[ 0 ] & 0xFFF );
		if ( !glove )
			return;

		GloveSkin_t skin = GetGloveSkin( g_ctx.local( )->GetTeam( ) == TeamID::TEAM_TERRORIST );

		if ( glove->m_iItemDefinitionIndex( ) != skin.index )
		{

			glove->m_iItemDefinitionIndex( ) = skin.index;

			glove->m_nFallbackSeed( ) = 0;
			glove->m_nFallbackStatTrak( ) = -1;
			glove->m_iEntityQuality( ) = 4;
			glove->m_iItemIDHigh( ) = -1;

			glove->m_nFallbackPaintKit( ) = skin.paint;

			if ( NeedsUpdate )
			{
				glove->GetClientNetworkable( )->PreDataUpdate( DATA_UPDATE_CREATED );
				glove->m_iAccountID( ) = localplayer_info.xuid_low;
				glove->set_model_index( m_modelinfo( )->GetModelIndex( "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl" ) );
				glove->m_nFallbackPaintKit( ) = CMenu::get()->index_knife;
				NeedsUpdate = false;
			}
		}
	}
}