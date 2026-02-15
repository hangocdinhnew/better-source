//========= Copyright 1996-2009, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
// This module implements the voice record and compression functions

//#include "audio_pch.h"
//#include "voice.h"
#include "tier0/platform.h"
#include "ivoicerecord.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"

#include <assert.h>
#include "SDL3/SDL.h"

// ------------------------------------------------------------------------------
// VoiceRecord_SDL
// ------------------------------------------------------------------------------
class VoiceRecord_SDL : public IVoiceRecord
{
protected:
	virtual				~VoiceRecord_SDL();
public:
	VoiceRecord_SDL();
	virtual void		Release();
	virtual bool		RecordStart();
	virtual void		RecordStop();

	// Initialize. The format of the data we expect from the provider is
	// 8-bit signed mono at the specified sample rate.
	virtual bool		Init(int sampleRate);
	virtual void		Idle() {}; // Stub
	void RenderBuffer( char *pszBuf, int size );

	// Get the most recent N samples.
	virtual int			GetRecordedData(short *pOut, int nSamplesWanted );

	int m_nSampleRate;
private:
	bool				InitalizeInterfaces();	// Initialize the openal capture buffers and other interfaces
	void				ReleaseInterfaces();	// Release openal buffers and other interfaces
private:

	SDL_AudioStream* m_Stream;
};

VoiceRecord_SDL::VoiceRecord_SDL() :
	m_nSampleRate( 0 ), m_Stream( nullptr )
{
}

VoiceRecord_SDL::~VoiceRecord_SDL()
{
	ReleaseInterfaces();
}

void VoiceRecord_SDL::Release()
{
	delete this;
}

bool VoiceRecord_SDL::RecordStart()
{
    if (!m_Stream)
        return InitalizeInterfaces();

    return true;
}

void VoiceRecord_SDL::RecordStop()
{
    if (m_Stream)
    {
        SDL_DestroyAudioStream(m_Stream);
        m_Stream = nullptr;
    }
}

bool VoiceRecord_SDL::InitalizeInterfaces()
{
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
		return false;

	SDL_AudioSpec desiredRecordingSpec;
	SDL_zero(desiredRecordingSpec);
	desiredRecordingSpec.freq = m_nSampleRate;
	desiredRecordingSpec.format = SDL_AUDIO_S16;
	desiredRecordingSpec.channels = 1;

	m_Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING,
					     &desiredRecordingSpec,
					     NULL,
					     NULL
					     );

	if(!m_Stream)
		return false;

	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(m_Stream));

	return true;
}

bool VoiceRecord_SDL::Init(int sampleRate)
{
	m_nSampleRate = sampleRate;
	ReleaseInterfaces();

	return true;
}


void VoiceRecord_SDL::ReleaseInterfaces()
{
	if (m_Stream)
		{
			SDL_DestroyAudioStream(m_Stream);
			m_Stream = nullptr;
		}
}


void VoiceRecord_SDL::RenderBuffer( char *pszBuf, int size )
{
}

int VoiceRecord_SDL::GetRecordedData(short *pOut, int nSamples)
{
    if (!m_Stream || nSamples <= 0)
        return 0;

    int bytesRequested = nSamples * sizeof(Sint16);

    int bytesAvailable = SDL_GetAudioStreamAvailable(m_Stream);
    if (bytesAvailable <= 0)
        return 0;

    int bytesToRead = SDL_min(bytesRequested, bytesAvailable);

    int bytesRead = SDL_GetAudioStreamData(
        m_Stream,
        (void*)pOut,
        bytesToRead
    );

    if (bytesRead < 0)
        return 0;

    return bytesRead / sizeof(Sint16);
}

IVoiceRecord* CreateVoiceRecord_SDL(int sampleRate)
{
	VoiceRecord_SDL *pRecord = new VoiceRecord_SDL;
	if ( pRecord && pRecord->Init(sampleRate) )
		return pRecord;
	else if( pRecord )
		pRecord->Release();

	return NULL;
}
