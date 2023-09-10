#pragma once
#include "..\..\includes.hpp"

class shared_data : public singleton <shared_data>
{
public:

	void setup(C_VoiceCommunicationData* data_output, C_CLCMsg_VoiceData* data_input);
	void share_data(C_VoiceCommunicationData* data_output, size_t m_chache_size, C_CLCMsg_VoiceData* data_input);
	void close_data(C_CLCMsg_VoiceData* data);
};