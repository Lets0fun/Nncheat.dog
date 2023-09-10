#include "discord_rpc_init.h"
#include <ctime>
#include "../Configuration/Config.h"
#include "utils/csgo.hpp"
#include "..\sdk\interfaces\IVEngineClient.hpp"
#include "..\includes.hpp"
#include "steam/steam_api.h"

void Discord::Initialize()
{
    DiscordEventHandlers Handle;
    memset(&Handle, 0, sizeof(Handle));
    Discord_Initialize("1146170079109718186", &Handle, 1, NULL);
}

void Discord::Update()
{
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    static auto elapsed = std::time(nullptr);

    std::string current_status = "in: main menu";
    if (m_engine( )->IsInGame( ) && m_engine( )->IsConnected( )) {
        current_status = "loading into game... ";
    }
    else if (m_engine( )->IsInGame( )) {
        current_status = "playing on: ";
        current_status += m_engine()->GetLevelName( );
    }

    discordPresence.details = "Playing With GameSense.Dog";
    discordPresence.state = "Get Good Get GS.DOG";
    discordPresence.state = current_status.c_str();
   // discordPresence.startTimestamp = elapsed;
    discordPresence.largeImageKey = "gslogo";
    discordPresence.smallImageKey = "v4";
    discordPresence.button1_label = "Website";
    discordPresence.button1_url = "https://gamesense.dog/";
    discordPresence.button2_label = "Discord";
    discordPresence.button2_url = "https://discord.gg/Jz3yNDQayz";
    Discord_UpdatePresence(&discordPresence);

}