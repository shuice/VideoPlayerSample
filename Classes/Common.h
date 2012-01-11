/*
 *  Common.h
 *  FFmpegTest
 *
 *  Create by xiaoyi liao  on 11-2-25.
 *  Copyright 2011 . All rights reserved.
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#define __STDC_LIMIT_MACROS
#include    <stdint.h>
#include    <inttypes.h>
#include    <math.h>
#include    <limits.h>
#import     <pthread.h>
#import     <unistd.h>
#include    <assert.h>
#include    <string>
#include    <vector>
using namespace std;

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#ifdef __cplusplus
extern "C" 
{
#endif
	
#include "libavutil/avutil.h"
#include "libavutil/common.h"
#include "libavutil/avstring.h"
//#include "libavutil/pixdesc.h"
//#include "libavcore/imgutils.h"
//#include "libavcore/parseutils.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavcodec/opt.h"
#include "libavcodec/audioconvert.h"
//#include "libavcodec/avfft.h"

extern void AddLog(const char* pszFormat, ...);
typedef void* (thread_func_t)(void*);
extern pthread_t StartThread(thread_func_t func, void *param);
extern void AudioCallback(void *opaque, unsigned char *stream, int len);
	
#ifdef __cplusplus
}
#endif

// DEL #include <SDL.h>
// DEL #include <SDL_thread.h>

enum EnumPlayerType
{
	ePlayerTypeUnkown = 0,
	ePlayerTypeLocal,
	ePlayerTypeLan,
	ePlayerTypeStream,
};

typedef EnumPlayerType EnumMediaType;

#ifdef __cplusplus
class CRect
{
public:
	CRect(int iLeft = 0, int iTop = 0, int iRight = 0, int iBottom = 0)
	{
		left = iLeft;
		top = iTop;
		right = iRight;
		bottom = iBottom;
	}
	unsigned long Width() const
	{
		unsigned long width = right - left;
		return width;
	}
	unsigned long Height() const
	{
		unsigned long height = bottom - top;
		return height;
	}
	
	unsigned long left;
	unsigned long top;
	unsigned long right;
	unsigned long bottom;
	
	
};
extern bool IsRectEmpty(const CRect* pRect);
extern bool RectStretchAdapt(const CRect* pInnerRect, const CRect* pOuterRect, CRect* pAdaptRect);
extern const char*  FormatTime(unsigned long iSecond, bool bPositive, char* pszTime, int iLen);


#endif
//#include "cmdutils.h"


#endif
