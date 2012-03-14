/*
 *  AudioLocalPlayer.cpp
 *  FFmpegTest
 *
 *  Create by xiaoyi liao on 11-2-24.
 *  Copyright 2011 . All rights reserved.
 *
 */

#include "AVLocalPlayer.h"
#import "PlayerView.h"
#import "PlayerViewControllerImp.h"
#import <AudioToolbox/AudioToolbox.h>

void RouteChangeListener(	void *                  inClientData,
						 AudioSessionPropertyID	inID,
						 UInt32                  inDataSize,
						 const void *            inData);
void interruptionListener(	void *	inClientData,
						  UInt32	inInterruptionState);

ALvoid  alBufferDataStaticProc(const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq);


void interruptionListener(	void *	inClientData,
						  UInt32	inInterruptionState)
{
	CAudioLocalPlayerSDL* pAudioLocalPlayerSDL = (CAudioLocalPlayerSDL*)inClientData;
	if (inInterruptionState == kAudioSessionBeginInterruption)
	{
		alcMakeContextCurrent(NULL);
	}
	else if (inInterruptionState == kAudioSessionEndInterruption)
	{
//		OSStatus result = AudioSessionSetActive(true);
//		if (result) 
//		{
//			NSLog(@"Error setting audio session active! %d\n", result);
//		}		
		alcMakeContextCurrent(pAudioLocalPlayerSDL->m_pALCContext);
	}
}


void RouteChangeListener(	void *                  inClientData,
						 AudioSessionPropertyID	inID,
						 UInt32                  inDataSize,
						 const void *            inData)
{
//	CFDictionaryRef dict = (CFDictionaryRef)inData;
//	
//	CFStringRef oldRoute = CFDictionaryGetValue(dict, CFSTR(kAudioSession_AudioRouteChangeKey_OldRoute));
//	
//	UInt32 size = sizeof(CFStringRef);
//	
//	CFStringRef newRoute;
//	OSStatus result = AudioSessionGetProperty(kAudioSessionProperty_AudioRoute, &size, &newRoute);
//	
//	NSLog(@"result: %d Route changed from %@ to %@", result, oldRoute, newRoute);
}
typedef ALvoid	AL_APIENTRY	(*alBufferDataStaticProcPtr) (const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq);

ALvoid  alBufferDataStaticProc(const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq)
{
	static	alBufferDataStaticProcPtr	proc = NULL;
    
    if (proc == NULL) {
        proc = (alBufferDataStaticProcPtr) alcGetProcAddress(NULL, (const ALCchar*) "alBufferDataStatic");
    }
    
    if (proc)
        proc(bid, format, data, size, freq);
	
    return;
}

CAudioLocalPlayerSDL::CAudioLocalPlayerSDL()
{
	m_pContext = NULL;	
	m_pALCDevice = NULL;
	m_pALCContext = NULL;
	memset(m_buffer, 0, sizeof(m_buffer));
	m_source = 0;
	m_iChannel = 0;
	m_tidPlayingAudio = 0;
	m_bAbort = false;
	m_iSampleRate = 0;
	m_iSizePerCall = 0;
}

void CAudioLocalPlayerSDL::SetVolume(int iVolume)
{
	alSourcef(m_source, AL_GAIN, iVolume * 1.0 / 100);
}

bool CAudioLocalPlayerSDL::Init(int iChannel, int iSampleRate, int iSizePerCall, void* pContext)
{
   //if (iSizePerCall == 0)
    {
        iSizePerCall = iSampleRate*iChannel / 10;
    }
	m_iSizePerCall = iSizePerCall;
	m_pContext = pContext;
	m_iSampleRate = iSampleRate;
	
	OSStatus result = AudioSessionInitialize(NULL, NULL, interruptionListener, this);
	if ((result != 0) && (result != kAudioSessionAlreadyInitialized))
	{
        NSLog(@"AudioSessionInitialize return %lu", (unsigned long)result);
        return false;
	}
	else
	{
		// if there is other audio playing, we don't want to play the background music
		int iPodIsPlaying = 0;
		UInt32 size = sizeof(iPodIsPlaying);
		result = AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, 
										 &size, 
										 &iPodIsPlaying);
		if (result)
		{
            NSLog(@"AudioSessionInitialize return %lu", (unsigned long)result);
			return false;
		}
		
		// if the iPod is playing, use the ambient category to mix with it
		// otherwise, use solo ambient to get the hardware for playing the app background track
		UInt32 category = kAudioSessionCategory_MediaPlayback;//(iPodIsPlaying) ? kAudioSessionCategory_AmbientSound : kAudioSessionCategory_SoloAmbientSound;
		result = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, 
										 sizeof(category), 
										 &category);		
		result = AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, 
												 RouteChangeListener, 
												 this);

		
		result = AudioSessionSetActive(true);
		if (result)
		{
            NSLog(@"AudioSessionInitialize return %lu", (unsigned long)result);
			return false;
		}
	}

	ALenum error = 0;
	m_pALCDevice = alcOpenDevice(NULL);
	if (m_pALCDevice == NULL)
	{
		//throw CPlayerException("alcOpenDevice return NULL");
		return true;
		return false;
	}

	m_pALCContext = alcCreateContext(m_pALCDevice, NULL);
	alcMakeContextCurrent(m_pALCContext);

	alGenBuffers(NUMBER_BUFFER, m_buffer);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		NSLog(@"AudioSessionInitialize return %lu", (unsigned long)result);
		return false;
	}
	
	alGenSources(1, &m_source);
	if(alGetError() != AL_NO_ERROR) 
	{
		NSLog(@"AudioSessionInitialize return %lu", (unsigned long)result);
		return false;
	}
	
	m_iChannel = iChannel;

	if (m_iChannel > 2)
	{
		m_iChannel = 2;
	}
        SetVolume(100);
	Play();
	return true;
}

void* CAudioLocalPlayerSDL::ThreadPlayingAudio(void* pParam)
{
	CAudioLocalPlayerSDL* pAudioLocalPlayerSDL = (CAudioLocalPlayerSDL*)pParam;
	if (pAudioLocalPlayerSDL == NULL)
	{
		return NULL;
	}
	return pAudioLocalPlayerSDL->ThreadPlayingAudioRoute();
}

void* CAudioLocalPlayerSDL::ThreadPlayingAudioRoute()
{

	ALenum eFormat = 0;
	if (m_iChannel == 1)
	{
		eFormat = alGetEnumValue("AL_FORMAT_MONO16");
	}
	else if	(m_iChannel == 2)
	{
		eFormat = alGetEnumValue("AL_FORMAT_STEREO16");
	}
	
	// 250ms
	unsigned char* pStream = new unsigned char[m_iSizePerCall];
	if (pStream == NULL)
	{
		NSLog(@"new audio stream buffer return NULL");
		return NULL;
	}
	
	for (int iIndex = 0; iIndex < NUMBER_BUFFER; iIndex ++) 
	{
		AudioCallback(m_pContext, pStream, m_iSizePerCall);	// get audio data from decode
		alBufferData(m_buffer[iIndex], eFormat, pStream, m_iSizePerCall, m_iSampleRate);
		alSourceQueueBuffers(m_source, 1, &m_buffer[iIndex]);
	}
	
	alSourcePlay(m_source);
	int error = 0;
	if((error = alGetError()) != AL_NO_ERROR) 
	{
		NSLog(@"error starting source: %x\n", error);
	} 
	while (!m_bAbort)  // big while
	{
		int iBuffersProcessed = 0;
		alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
		
		for ( ; iBuffersProcessed > 0; iBuffersProcessed --)
		{
			ALuint buffer = 0;
			alSourceUnqueueBuffers(m_source, 1, &buffer);
			AudioCallback(m_pContext, pStream, m_iSizePerCall);
			alBufferData(buffer, eFormat, pStream, m_iSizePerCall, m_iSampleRate);
			alSourceQueueBuffers(m_source, 1, &buffer);
		}
		
		int iState = 0;
		alGetSourcei(m_source, AL_SOURCE_STATE, &iState);
		if (iState != AL_PLAYING)
		{
			int iQueuedBuffers = 0;
			alGetSourcei(m_source, AL_BUFFERS_QUEUED, &iQueuedBuffers);
			if (iQueuedBuffers != 0)
			{
				alSourcePlay(m_source);
			}
			else
			{
			//	break;	// finish
			}

		}
		usleep(10000);
	}
	alSourceStop(m_source);
	alSourcei(m_source, AL_BUFFER, 0);
	delete[] pStream;
	pStream = NULL;
	return NULL;
}

void CAudioLocalPlayerSDL::Play()
{
	m_tidPlayingAudio = StartThread(ThreadPlayingAudio, this);
}

void CAudioLocalPlayerSDL::Pause()
{
	// nothing to do
}
void CAudioLocalPlayerSDL::Close()
{
	m_bAbort = true;
	pthread_join(m_tidPlayingAudio, NULL);
	alDeleteBuffers(NUMBER_BUFFER, m_buffer);
	alDeleteSources(1, &m_source);
	alcDestroyContext(m_pALCContext);
	alcCloseDevice(m_pALCDevice);
}


CVideoLocalPlayerSDL::CVideoLocalPlayerSDL()
{
	m_iSrcWidth = 0;
	m_iSrcHeight = 0;
	m_iDesWidth = 0;
	m_iDesHeight = 0;
    m_iWindow = 0;
	m_pSwsContext = NULL;
#if USE_RGB_TEXTURE
    m_pixelFormat = PIX_FMT_RGBA;
#else
    m_pixelFormat = PIX_FMT_YUV420P;
#endif
	memset(m_pRGBAData, 0, sizeof(m_pRGBAData));
}

CVideoLocalPlayerSDL::~CVideoLocalPlayerSDL()
{
	if (m_pSwsContext != NULL)
	{
		sws_freeContext(m_pSwsContext);
		m_pSwsContext = NULL;
	}
	for (int iCacheIndex = 0; iCacheIndex < MAX_CACHE_COUNT; iCacheIndex ++)
	{
		if (m_pRGBAData[iCacheIndex] != NULL)
		{
			avpicture_free(&m_avPicture[iCacheIndex]);
			pthread_mutex_destroy(&m_mutex[MAX_CACHE_COUNT]);
		}
	}
}

void CVideoLocalPlayerSDL::CalcDesSize()
{
    int iTextureWidth = TEXTURE_WIDTH;
    int iTextureHeight = TEXTURE_HEIGHT;
    m_iDesWidth = m_iSrcWidth;
    m_iDesHeight = m_iSrcHeight;
    if (m_iDesWidth > iTextureWidth)
    {
        m_iDesHeight /= (m_iDesWidth * 1.0 / iTextureWidth);
        m_iDesHeight -=  (m_iDesHeight&7);
        m_iDesWidth = iTextureWidth;
    }
    
    if (m_iDesHeight > iTextureHeight)
    {
        m_iDesWidth /= (m_iDesHeight * 1.0 / iTextureHeight);
        m_iDesWidth -=  (m_iDesWidth&7);
        m_iDesHeight = iTextureHeight;
    }
}

bool CVideoLocalPlayerSDL::Init(long iWindow, int iMediaWidth, int iMediaHeight, PixelFormat ePixelFormat, int iCacheCount)
{
	if (iCacheCount > MAX_CACHE_COUNT)
	{
		return false;
	}
    m_iWindow = iWindow;
	m_iSrcWidth = iMediaWidth;
    m_iSrcHeight = iMediaHeight;
    CalcDesSize();
    
    PlayerView* playerView = (PlayerView*)iWindow;
    [playerView setMovieSize:m_iSrcWidth iHeight:m_iSrcHeight iWidthResized:m_iDesWidth iHeightResized:m_iDesHeight];

    m_pSwsContext = sws_getContext(m_iSrcWidth, m_iSrcHeight, ePixelFormat, 
								   m_iDesWidth, m_iDesHeight, m_pixelFormat,
								   SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (m_pSwsContext == NULL) 
	{
		return false;
	}
	
	
	for (int iCacheIndex = 0; iCacheIndex < iCacheCount; iCacheIndex ++)
	{
		pthread_mutex_init(&m_mutex[iCacheIndex], NULL);
		avpicture_alloc(&m_avPicture[iCacheIndex], m_pixelFormat, m_iDesWidth, m_iDesHeight);
		m_pRGBAData[iCacheIndex] = (unsigned char*)m_avPicture[iCacheIndex].data[0];
	}
	return true;
}

bool saveBmp(const char* bmpName,unsigned char *imgBuf,int width,int height,int biBitCount);
void CVideoLocalPlayerSDL::UpdateData(AVFrame* pAvFrame, int iCacheIndex)
{	
	sws_scale(m_pSwsContext, pAvFrame->data, pAvFrame->linesize,
 			  0, m_iSrcHeight, m_avPicture[iCacheIndex].data, m_avPicture[iCacheIndex].linesize);   
}

void CVideoLocalPlayerSDL::Show(int iCacheIndex)
{
    PlayerView* playerView = (PlayerView*)m_iWindow;
    playerView->m_pData[0] = m_avPicture[iCacheIndex].data[0];
    playerView->m_pData[1] = m_avPicture[iCacheIndex].data[1];
    playerView->m_pData[2] = m_avPicture[iCacheIndex].data[2];
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSData* nsData = [NSData dataWithBytes:m_wstrSubTitle.c_str()
                                    length:m_wstrSubTitle.size()*sizeof(wchar_t)];
    NSString* str = [[[NSString alloc] initWithData:nsData 
                                          encoding:NSUTF32LittleEndianStringEncoding] autorelease];
    [playerView performSelectorOnMainThread:@selector(showWithSubTitle:) withObject:str waitUntilDone:NO];
//    [playerView performSelector:@selector(showWithSubTitle:) onThread:[NSThread mainThread] withObject:str waitUntilDone:NO];
    [pool release];
}

void CVideoLocalPlayerSDL::Close()
{
	PlayerView* playerView = (PlayerView*)m_iWindow;
    [playerView performSelectorOnMainThread:@selector(playFinish:) withObject:nil waitUntilDone:NO];
}

void CVideoLocalPlayerSDL::PlayFinish()
{
    
}

void AddLog(const char* pszFormat, ...)
{
    va_list vaList;
	va_start (vaList, pszFormat);
	char szDescription[1000] = {0};
	vsprintf(szDescription, pszFormat, vaList);
	va_end(vaList);
	NSLog(@"%s", szDescription);
}

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long LONG;

bool saveBmp(const char* bmpName,unsigned char *imgBuf,int width,int height,int biBitCount)
{
    if(!imgBuf)//imgBuf 待存盘的位图数据
        return 0;
    int colorTablesize = 0;
    if(biBitCount == 8)
        colorTablesize =1024;
    int lineByte = (width * biBitCount/8+3)/4*4;
    FILE *fp = fopen(bmpName,"w");
    if(fp == 0) return 0;
    BITMAPFILEHEADER fileHead;
    fileHead.bfType= 0x4d42;
    fileHead.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte *height;
    fileHead.bfReserved1 = 0;
    fileHead.bfReserved2 = 0;
    fileHead.bfOffBits = 54 +colorTablesize;
    fwrite(&fileHead,sizeof(BITMAPFILEHEADER),1,fp);
    BITMAPINFOHEADER head;
    head.biBitCount = biBitCount;
    head.biClrImportant = 0;
    head.biClrUsed = 0;
    head.biCompression = 0;
    head.biHeight = height;
    head.biPlanes =1;
    head.biSize = 40;
    head.biSizeImage = lineByte *height;
    head.biWidth = width;
    head.biXPelsPerMeter = 0;
    head.biYPelsPerMeter = 0;
    fwrite(&head,sizeof(BITMAPINFOHEADER),1,fp);
    
    fwrite(imgBuf,height * lineByte,1,fp);
    fclose(fp);
    return 1;
    
}