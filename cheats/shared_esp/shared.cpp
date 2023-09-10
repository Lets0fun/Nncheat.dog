#include "shared.hpp"


void shared_data::setup(C_VoiceCommunicationData* data_output, C_CLCMsg_VoiceData* data_input)
{

	this->share_data(data_output, 1024, data_input);
}

void shared_data::share_data(C_VoiceCommunicationData* data_output, size_t m_chache_size, C_CLCMsg_VoiceData* data_input)
{
	data_input->SetData(data_output);
	data_input->pData = data_output;
	data_input->m_nCachedSize = m_chache_size;
}

void shared_data::close_data(C_CLCMsg_VoiceData* data)
{
	data->pData = nullptr;
}