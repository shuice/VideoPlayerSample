/*
 * FFplay : Simple Media Player based on the FFmpeg libraries
 * Copyright (c) 2003 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include "Common.h"
#include "Common1.h"
#include "AVLocalPlayer.h"
#include "PacketQueue.h"
#include "pthread.h"


#include "SampleFmt.h"
#include "SrtSubTitleReader.h"
#include "SmiSubTitleReader.h"
#include "PlayerViewDefine.h"

#define MAX_QUEUE_SIZE (10 * 1024 * 1024)
#define MIN_AUDIOQ_SIZE (120 * 1024)
#define MIN_FRAMES 24

/* no AV sync correction is done if below the AV sync threshold */
#define AV_SYNC_THRESHOLD 0.01
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

#define FRAME_SKIP_FACTOR 0.05

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
#define SAMPLE_ARRAY_SIZE (2*65536)



#define VIDEO_PICTURE_QUEUE_SIZE 2



enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};
/**
 * all in native-endian format
 */

typedef struct {
    int64_t num_faulty_pts; /// Number of incorrect PTS values so far
    int64_t num_faulty_dts; /// Number of incorrect DTS values so far
    int64_t last_pts;       /// PTS of the last frame
    int64_t last_dts;       /// DTS of the last frame
} PtsCorrectionContext;

enum AVMediaType {
    AVMEDIA_TYPE_UNKNOWN = -1,
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO,
    AVMEDIA_TYPE_DATA,
    AVMEDIA_TYPE_SUBTITLE,
    AVMEDIA_TYPE_ATTACHMENT,
    AVMEDIA_TYPE_NB
};


typedef void SDL_Thread;
typedef struct VideoState 
{
	CVideoLocalPlayer* m_pVideoLocalPlayer;
	CAudioLocalPlayer* m_pAudioLocalPlayer;
	string m_strFileName;
	SMediaInfoStream sMediaInfoStream;
    pthread_t read_tid;
    pthread_t video_tid;
    pthread_t refresh_tid;
	pthread_t event_tid;
    AVInputFormat *iformat;
    bool abort_request;
    bool paused;
    int last_paused;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int read_pause_return;
    AVFormatContext *ic;

	
    int audio_stream;
	
    int av_sync_type;
    double external_clock; /* external clock base */
    int64_t external_clock_time;
	
    double audio_clock;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
	CPacketQueue audioq;
	int64_t audio_callback_time;
    int audio_hw_buf_size;
 	uint8_t __attribute__ ((aligned (16))) audio_buf1[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	uint8_t __attribute__ ((aligned (16))) audio_buf2[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];


    uint8_t *audio_buf;
    unsigned int audio_buf_size; /* in bytes */
    int audio_buf_index; /* in bytes */
    AVPacket audio_pkt_temp;
    AVPacket audio_pkt;
    enum SampleFormat audio_src_fmt;
    AVAudioConvert *reformat_ctx;
	
	
    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;

	
    double frame_timer;
    double frame_last_pts;
    double frame_last_delay;
    double video_clock;                          ///<pts of last decoded frame / predicted pts of next decoded frame
    int video_stream;
    AVStream *video_st;
    CPacketQueue videoq;
    double video_current_pts;                    ///<current displayed pts (different from video_clock if frame fifos are used)
    double video_current_pts_drift;              ///<video_current_pts - time (av_gettime) at which we updated video_current_pts - used to have running video pts
    int64_t video_current_pos;                   ///<current displayed file pos
    double pts[VIDEO_PICTURE_QUEUE_SIZE];
	double targetTime[VIDEO_PICTURE_QUEUE_SIZE];
    int pictq_size, pictq_rindex, pictq_windex;
	pthread_mutex_t* m_pMutexPictQueue;
	pthread_cond_t* m_pConditionPictQueue;

    struct SwsContext *img_convert_ctx;

	
    //    QETimer *video_timer;
    int width, height;
	
    PtsCorrectionContext pts_ctx;
	
    bool m_bRefreshed;
	int step;
	long iWindow;
	char* m_pBufferReaded;
	AVCodecContext* m_pCodecContextAudio;
	AVCodecContext* m_pCodecContextVideo;
    EnumPlaySpeed m_ePlaySpeed;
} VideoState;
typedef VideoState SVideoState;


#pragma mark CLocalPlayer



class CLocalPlayer : private VideoState
{
public:
	CLocalPlayer();
	~CLocalPlayer();
	EnumPlayerStatus Open(const string& strFileName, long iWindow);
	bool Start();
	bool Play();
	bool Pause();
	void Close();
	void Step();
	void SeekBySecond(int iSecond);
	void SetVolume(int iVolume);
	bool HasVideo();
	bool IsPaused();
    void SetPlaySpeed(EnumPlaySpeed ePlaySpeed);
    EnumPlaySpeed GetPlaySpeed();
	
	void AudioCallBack(unsigned char *stream, int len);
	unsigned long m_iCurrentTime;
	unsigned long m_iDuration;
	bool	m_bReadEndOfFile;
   
	
	int socketCommand;
	int socketCache;
	
	string m_strSubTitleLocal;
	int m_iCodePage;
//	string 
private:
	CSrtSubTitleReader* m_pSubTitleReader;
private:
	// open invoked
	EnumPlayerStatus InitData(const string& strFileName); // 初始化播放器数据
	EnumPlayerStatus InitUIAndAudioDevice(); // 初始化播放器界面和音频设备
	static int DecodeInterruptCallBack();
	int OpenStream(int stream_index);
	static void* ThreadVideo(void* pParam);
	void* ThreadVideoRoute();
	int OutputPicture(AVFrame *src_frame, double pts1);
	int QueuePicture(AVFrame *src_frame, double pts);
	static void* ThreadRefresh(void* pParam);
	void* ThreadRefreshRoute();
	void VideoImageDisplay();
	double GetAudioClock();
	double GetVideoClock();
	double GetExternalClock();
	double GetMasterClock();
	void VideoDisplay();
	void StreamSeek(int64_t pos);
	void StreamPause();
	void VideoAudioDisplay();
	int SynchronizeAudio(short *samples, int samples_size1, double pts);
	int AudioDecodeFrame(double *pts_ptr);
	void StreamComponentClose(int stream_index);
	static void* ThreadRead(void* pParam);
	void* ThreadReadRoute();
	int AudioWriteGetBufSize();
	double ComputeTargetTime(double frame_current_pts);
	
	static inline int ComputeMod(int a, int b);
	static bool m_bDecodeAbortRequest;
};