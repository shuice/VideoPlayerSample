/*
 *  AudioLocalPlayer.h
 *  FFmpegTest
 *
 *  Create by xiaoyi liao on 11-2-24.
 *  Copyright 2011 . All rights reserved.
 *
 */
#pragma once
#include "common.h"
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
class CAudioLocalPlayer
{
public:
	virtual bool Init(int iChannel, int iSampleRate, int iSizePerCall, void* pContext) = 0;
	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual void Close() = 0;
	virtual void SetVolume(int iVolume) = 0;
};


class CVideoLocalPlayer
{
public:
	virtual bool Init(long iWindow, int iMediaWidth, int iMediaHeight, PixelFormat ePixelFormat, int iCacheIndex) = 0;
	virtual void UpdateData(AVFrame* pAvFrame, int iCacheIndex) = 0;
	virtual void Show(int iCacheIndex) = 0;
	virtual void Close() = 0;
    virtual void PlayFinish() = 0;
	wstring m_wstrSubTitle;	// BAD
};

#define CAudioLocalPlayerImp CAudioLocalPlayerSDL

class CAudioLocalPlayerSDL : public CAudioLocalPlayer
{
public:
	CAudioLocalPlayerSDL();
	virtual bool Init(int iChannel, int iSampleRate, int iSizePerCall, void* pContext);
	virtual void Play();
	virtual void Pause();
	virtual void Close();
	virtual void SetVolume(int iVolume);
	ALCcontext* m_pALCContext;
private:
	static void* ThreadPlayingAudio(void* pParam);
	void* ThreadPlayingAudioRoute();
	void* m_pContext;
	
	
	ALCdevice* m_pALCDevice;

#define NUMBER_BUFFER 4
	ALuint m_buffer[NUMBER_BUFFER];
	ALuint m_source;
	int		m_iChannel;
	int		m_iSampleRate;
	pthread_t m_tidPlayingAudio;
	bool m_bAbort;
	int m_iSizePerCall;
};

#define CVideoLocalPlayerImp CVideoLocalPlayerSDL
#define MAX_CACHE_COUNT 2
class CVideoLocalPlayerSDL : public CVideoLocalPlayer
{
public:
	CVideoLocalPlayerSDL();
	~CVideoLocalPlayerSDL();
	virtual bool Init(long iWindow, int iMediaWidth, int iMediaHeight, PixelFormat ePixelFormat, int iCacheCount);
	virtual void UpdateData(AVFrame* pAvFrame, int iCacheIndex);
	virtual void Show(int iCacheIndex);
	virtual void Close();
    virtual void PlayFinish();
private:
    void CalcDesSize();
	unsigned char* m_pRGBAData[MAX_CACHE_COUNT];
	AVPicture m_avPicture[MAX_CACHE_COUNT];
	pthread_mutex_t m_mutex[MAX_CACHE_COUNT];
	
	int m_iSrcWidth;
	int m_iSrcHeight;
	int m_iDesWidth;
	int m_iDesHeight;
    PixelFormat m_pixelFormat;

	SwsContext* m_pSwsContext;
    int m_iWindow;
    AVPicture pic;
};