#pragma once

#define NDEBUG
#define DIRECTINPUT_VERSION 0x0800

#include "version.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <deque>
#include <array>
#include <ctime>

#include <d3d9.h>
#include <d3dx9.h>
#include "Configuration/Config.h"

#include <dinput.h>
#include <tchar.h>

#include "cheats\misc\key_binds.h"
#include "utils\util.hpp"
#include "utils\csgo.hpp"
#include "utils\draw_manager.h"
#include "utils\ctx.hpp"
#include "utils\netmanager.hpp"
#include "utils\math.hpp"
#include "utils\singleton.h"
#include "utils\md5.h"
#include "utils\crypt_str.h"

#include "lua\Clua.h"
#include "detours.h"
#include "hooks\hooks.hpp"
#include "configs\configs.h"

#include "resources\custom_sounds.hpp"
#include "resources\sounds.hpp"

#include "sdk\math\Vector.hpp"
#include "sdk\math\VMatrix.hpp"

#include "sdk\misc\UtlVector.hpp"
#include "sdk\misc\EHandle.hpp"
#include "sdk\misc\CUserCmd.hpp"
#include "sdk\misc\Color.hpp"
#include "sdk\misc\KeyValues.hpp"
#include "sdk\misc\datamap.h"

#include "sdk\interfaces\IClientEntity.hpp"
#include "sdk\structs.hpp"


class C_VoiceCommunicationData
{
public:
	uint32_t m_nXuidLow{ };
	uint32_t m_nXuidHigh{ };
	int32_t m_nSequenceBytes{ };
	uint32_t m_nSectionNumber{ };
	uint32_t m_nUnCompressedSampleOffset;

	__forceinline uint8_t* GetRawData()
	{
		return  (uint8_t*)this;
	}

};

class C_CLCMsg_VoiceData
{
public:
	uint32_t INetMessageVTable; //0x0000
	char PAD[4]; //0x0004
	uint32_t CCLCMsg_VoiceData_Vtable; //0x0008
	char PAD0C[8]; //0x000C
	void* pData; //0x0014
	uint32_t m_nXuidLow{ };
	uint32_t m_nXuidHigh{ };
	int32_t m_nFormat; //0x0020
	int32_t m_nSequenceBytes; //0x0024
	uint32_t m_nSectionNumber; //0x0028
	uint32_t m_nUnCompressedSampleOffset; //0x002C
	int32_t m_nCachedSize; //0x0030

	uint32_t m_nFlags; //0x0034

	uint8_t NoStackOverFlow[0xFF]; // fix clown crashers

	__forceinline void SetData(C_VoiceCommunicationData* pData)
	{
		m_nXuidLow = pData->m_nXuidLow;
		m_nXuidHigh = pData->m_nXuidHigh;
		m_nSequenceBytes = pData->m_nSequenceBytes;
		m_nSectionNumber = pData->m_nSectionNumber;
		m_nUnCompressedSampleOffset = pData->m_nUnCompressedSampleOffset;
	}
};
class C_SVCMsg_GameEvent
{
public:
	char PAD[0x8];
};
class C_SVCMsg_VoiceData
{
public:
	char PAD[0x8];
	int m_iClient;
	int m_nAudibleMask;
	uint32_t m_nXuidLow{ };
	uint32_t m_nXuidHigh{ };
	std::string* m_szVoiceData;
	bool m_bProximity;
	bool m_bCaster;
	int m_nFormat;
	int m_nSequenceBytes;
	uint32_t m_nSectionNumber;
	uint32_t m_nUnCompressedSampleOffset;

	__forceinline C_VoiceCommunicationData GetData()
	{
		C_VoiceCommunicationData cData;
		cData.m_nXuidLow = m_nXuidLow;
		cData.m_nXuidHigh = m_nXuidHigh;
		cData.m_nSequenceBytes = m_nSequenceBytes;
		cData.m_nSectionNumber = m_nSectionNumber;
		cData.m_nUnCompressedSampleOffset = m_nUnCompressedSampleOffset;
		return cData;
	}
};
class C_SVCMsg_SoundData
{
public:
	char PAD[0x8];
	void* m_pAllocation;
	int m_nSize;
};

#define m_client g_csgo.m_client
#define m_clientmode g_csgo.m_clientmode
#define m_clientstate g_csgo.m_clientstate
#define m_cvar g_csgo.m_cvar
#define m_debugoverlay g_csgo.m_debugoverlay
#define m_device g_csgo.m_device
#define m_engine g_csgo.m_engine
#define m_enginesound g_csgo.m_enginesound
#define m_entitylist g_csgo.m_entitylist
#define m_eventmanager g_csgo.m_eventmanager
#define m_gamemovement g_csgo.m_gamemovement
#define m_gamerules g_csgo.m_gamerules
#define m_globals g_csgo.m_globals
#define m_glow g_csgo.m_glow
#define m_input g_csgo.m_input
#define m_inputinternal g_csgo.m_inputinternal
#define m_inputsys g_csgo.m_inputsys
#define m_localize g_csgo.m_localize
#define m_materialsystem g_csgo.m_materialsystem
#define m_memalloc g_csgo.m_memalloc
#define m_modelcache g_csgo.m_modelcache
#define m_modelinfo g_csgo.m_modelinfo
#define m_modelrender g_csgo.m_modelrender
#define m_movehelper g_csgo.m_movehelper
#define m_panel g_csgo.m_panel
#define m_physsurface g_csgo.m_physsurface
#define m_playerresource g_csgo.m_playerresource
#define m_postprocessing g_csgo.m_postprocessing
#define m_prediction g_csgo.m_prediction
#define m_renderview g_csgo.m_renderview
#define m_surface g_csgo.m_surface
#define m_trace g_csgo.m_trace
#define m_viewrenderbeams g_csgo.m_viewrenderbeams
#define m_soundservices g_csgo.m_soundservices
#define m_basefilesys g_csgo.m_basefilesys
#define m_weaponsys g_csgo.m_weaponsys
#define m_networkStringTableContainer g_csgo.m_networkStringTableContainerr