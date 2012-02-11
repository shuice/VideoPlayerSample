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

#include "LocalPlayer.h"
#include "PlayerException.h"
#include <fcntl.h>

bool CLocalPlayer::m_bDecodeAbortRequest = false;
CLocalPlayer::CLocalPlayer()
{
	m_pCurrentTime = NULL;
	m_pDuration = NULL;
	
	m_pVideoLocalPlayer = NULL;
	m_pAudioLocalPlayer = NULL;
	read_tid = NULL;
	video_tid = NULL;
	refresh_tid = NULL;
	event_tid = NULL;
	iformat = NULL;
	abort_request = false;
	paused = false;
	last_paused = 0;
	seek_req = 0;
	seek_flags = 0;
	seek_pos = 0;
	read_pause_return = 0;
	ic = NULL;	
		
	audio_stream = -1;
	av_sync_type = AV_SYNC_AUDIO_MASTER;
	external_clock = 0.0;
	external_clock_time = 0.0;
		
	audio_clock = 0.0;
	audio_diff_cum = 0.0; 
	audio_diff_avg_coef = 0.0;
	audio_diff_threshold = 0.0;
	audio_diff_avg_count = 0;
	audio_st = NULL;
	audio_hw_buf_size = 0;
	audio_callback_time = 0;

	memset(audio_buf1, 0, sizeof(audio_buf1));
	memset(audio_buf2, 0, sizeof(audio_buf2));
	audio_buf = 0;
	audio_buf_size = 0; /* in bytes */
	audio_buf_index = 0; /* in bytes */
	av_init_packet(&audio_pkt_temp);
	av_init_packet(&audio_pkt);
	
	audio_src_fmt = SAMPLE_FMT_NONE;
	
		
	memset(sample_array, 0, sizeof(sample_array));
	sample_array_index = 0;
	last_i_start = 0;

	frame_timer = 0.0;
	frame_last_pts = AV_NOPTS_VALUE;
	frame_last_delay = 0.0;
	video_clock = 0.0; 
	video_stream = 0;
	video_st = NULL;
	video_current_pts = 0.0;
	video_current_pts_drift = 0.0;
	video_current_pos = 0;
	

	pictq_size = 0;
	pictq_rindex = 0;
	pictq_windex = 0;
	reformat_ctx = NULL;
	m_pBufferReaded = NULL;
	m_pCodecContextVideo = NULL;

	m_pMutexPictQueue = new pthread_mutex_t;
    if (m_pMutexPictQueue == NULL)
    {
        throw new CPlayerException("new pthread_mutex_t return NULL");
    }
    pthread_mutex_init(m_pMutexPictQueue, NULL);
	
    m_pConditionPictQueue = new pthread_cond_t;
	if (m_pConditionPictQueue == NULL)
	{
		throw new CPlayerException("new pthread_cond_t return NULL");
	}
    pthread_cond_init(m_pConditionPictQueue, NULL);
	
	m_pBufferReaded = new char[512 * 1024];
	if (m_pBufferReaded == NULL)
	{
		throw new CPlayerException("new m_pBufferReaded return NULL");
	}
    
	img_convert_ctx = NULL;
		
		
	width = 0;
	height = 0;		
	memset(&pts_ctx, 0, sizeof(pts_ctx));
		
	m_bRefreshed = true;
	step = 0;
	iWindow = 0;
    
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();
}

CLocalPlayer::~CLocalPlayer()
{
	pthread_mutex_destroy(m_pMutexPictQueue);
 	if (m_pMutexPictQueue != NULL)
	{
		delete m_pMutexPictQueue;
		m_pMutexPictQueue = NULL;
	}
	pthread_cond_destroy(m_pConditionPictQueue);
	if (m_pConditionPictQueue != NULL)
	{
		delete m_pConditionPictQueue;
		m_pConditionPictQueue = NULL;
	}
}

// sFileName is filekey
bool CLocalPlayer::Open(const string& strFileName, long iWindow1)
{
	if (!InitData(strFileName))
	{
		return false;
	}
	iWindow = iWindow1;
	if (!InitUIAndAudioDevice())
	{
		return false;
	}
	m_srtSubTitleReader.LoadFile(m_strSubTitleLocal);
	m_srtSubTitleReader.SetCodePage(m_iCodePage);
	return true;
}

bool CLocalPlayer::Start()
{
	refresh_tid = StartThread(ThreadRefresh, this);
    video_tid = StartThread(ThreadVideo, this);
	read_tid = StartThread(ThreadRead, this);
	return true;
}

bool CLocalPlayer::Pause()
{
	StreamPause();
	step = 0;
	return true;
}

bool CLocalPlayer::Play()
{
	Pause();
	return true;
}

void CLocalPlayer::Close()
{

	audioq.Abort();
	audioq.End();
	// close audio
	if (m_pAudioLocalPlayer != NULL)
	{
		m_pAudioLocalPlayer->Close();
		delete m_pAudioLocalPlayer;
		m_pAudioLocalPlayer = NULL;
	}

	abort_request = 1;

	// close stream and thread
	
	//printf("\n---------------------------------------------Close Wait Read");
	pthread_join(read_tid, NULL);
	pthread_join(refresh_tid, NULL);
	
	
    /* free all pictures */
    pthread_mutex_destroy(m_pMutexPictQueue);
    pthread_cond_destroy(m_pConditionPictQueue);
	if (img_convert_ctx)
	{
		sws_freeContext(img_convert_ctx);
	}
	
	// close video
	if (m_pVideoLocalPlayer != NULL)
	{
		m_pVideoLocalPlayer->Close();
		delete m_pVideoLocalPlayer;
		m_pVideoLocalPlayer = NULL;
	}
	//printf("\n---------------------------------------------Close End");
}


bool CLocalPlayer::HasVideo()
{
	if (m_ePlayerType == ePlayerTypeStream)
	{
		return sMediaInfoStream.m_bHasVideo;
	}
	else
	{
		if (video_st != NULL)
		{
			return true;
		}
	}
	return false;
}

int CLocalPlayer::DecodeInterruptCallBack()
{
	return m_bDecodeAbortRequest;
}

bool CLocalPlayer::InitData(const string& strFileName)
{
	m_strFileName = strFileName;
	
	{
	    AVFormatContext* pAvFormatContext = avformat_alloc_context();
		if (pAvFormatContext == NULL)
		{
			throw new CPlayerException("avformat_alloc_context return NULL");
		}
		int st_index[AVMEDIA_TYPE_NB];
		for (int iStIndex = 0; iStIndex < AVMEDIA_TYPE_NB; iStIndex ++)
		{
			st_index[iStIndex] = -1;
		}
	    video_stream = -1;
	    audio_stream = -1;
	
	    url_set_interrupt_cb(DecodeInterruptCallBack);
	
		AVFormatParameters avFormatParameters = {0};	
	    avFormatParameters.prealloced_context = 1;
		avFormatParameters.width = 640;
		avFormatParameters.height= 480;
	    avFormatParameters.time_base= (AVRational){1, 25};
		avFormatParameters.pix_fmt = PIX_FMT_NONE;
	
	//	set_context_opts(pAvFormatContext, avformat_opts, AV_OPT_FLAG_DECODING_PARAM, NULL);
	    int iRet = av_open_input_file(&pAvFormatContext, strFileName.c_str(), 
									  iformat, 0, &avFormatParameters);
	    if (iRet < 0) 
		{
			throw new CPlayerException("av_open_input_file return false", m_strFileName.c_str());
	    }
	    ic = pAvFormatContext;

		//    if(genpts)
		pAvFormatContext->flags |= AVFMT_FLAG_GENPTS;
	
	    iRet = av_find_stream_info(pAvFormatContext);
	    if (iRet < 0)
		{
	        throw new CPlayerException("%s: could not find codec parameters", m_strFileName.c_str());
	    }
	    if(pAvFormatContext->pb)
		{
	        pAvFormatContext->pb->eof_reached= 0; //FIXME hack, ffplay maybe should not use url_feof() to test for the end
		}
		
	    for (int iStreamIndex = 0; iStreamIndex < pAvFormatContext->nb_streams; iStreamIndex++)
		{
	        pAvFormatContext->streams[iStreamIndex]->discard = AVDISCARD_ALL;
			// try find suitable stream
			AVCodecContext *pAvCodecContext = pAvFormatContext->streams[iStreamIndex]->codec;
	        switch(pAvCodecContext->codec_type)
			{
				case CODEC_TYPE_AUDIO:
					if (st_index[AVMEDIA_TYPE_AUDIO] == -1)
					{
						st_index[AVMEDIA_TYPE_AUDIO] = iStreamIndex;
					}
					break;
				case CODEC_TYPE_VIDEO:
					if (st_index[AVMEDIA_TYPE_VIDEO] == -1)
					{
						st_index[AVMEDIA_TYPE_VIDEO] = iStreamIndex;
					}
					break;
				default:
					break;
	        }
		
		}
    
		// try open stream
		if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) 
		{
			OpenStream(st_index[AVMEDIA_TYPE_AUDIO]);
		}
	    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) 
		{
	        OpenStream(st_index[AVMEDIA_TYPE_VIDEO]);
	    }
	
	    if (video_stream < 0 && audio_stream < 0) 
		{
	        throw new CPlayerException("can not find any valid streams");
			return false;
	    }
	
		*m_pDuration =  (unsigned long)pAvFormatContext->duration / AV_TIME_BASE;
	    *m_pCurrentTime = 0;
	}
	return true;
}

bool CLocalPlayer::InitUIAndAudioDevice()
{
	m_pAudioLocalPlayer = new CAudioLocalPlayerImp;
	if (m_pAudioLocalPlayer == NULL)
	{
		throw new CPlayerException("new CAudioLocalPlayerImp return NULL");
	}
	m_pVideoLocalPlayer = new CVideoLocalPlayerImp;
	if (m_pVideoLocalPlayer == NULL)
	{
		throw new CPlayerException("new CVideoLocalPlayerImp return NULL");
	}
	
	if (m_ePlayerType == ePlayerTypeStream)
	{
		if (!m_pAudioLocalPlayer->Init(2, 44100, 8192, this))
		{
			throw new CPlayerException("init audio error");
			return false;
		}

		if (!m_pVideoLocalPlayer->Init(iWindow, sMediaInfoStream.m_iWidthEncoded,
												 sMediaInfoStream.m_iHeightEncoded,
												 PIX_FMT_YUV420P,
												 VIDEO_PICTURE_QUEUE_SIZE))
		{
			throw new CPlayerException("init video error");
			return false;
		}
	}
	else
	{		
		if (audio_stream >= 0)
		{
			if (!m_pAudioLocalPlayer->Init(audio_st->codec->channels, 
													 audio_st->codec->sample_rate,
													 audio_st->codec->frame_size * 10,
														this))
			{
				throw new CPlayerException("init audio error");
				return false;
			}
		}
		if (video_stream >= 0)
		{
			if (!m_pVideoLocalPlayer->Init(iWindow,
													 video_st->codec->width,
													 video_st->codec->height,
													 video_st->codec->pix_fmt,
													 VIDEO_PICTURE_QUEUE_SIZE))
			{
				throw new CPlayerException("init video error");
				return false;
			}
		}
	}

	return true;
}


/* open a given stream. Return 0 if OK */
int CLocalPlayer::OpenStream(int stream_index)
{    
	{
	
	    if (stream_index < 0 || stream_index >= ic->nb_streams)
		{
	        return -1;
		}
	    AVCodecContext *pAvCodecContext = ic->streams[stream_index]->codec;
	    AVCodec *pAvCodec = avcodec_find_decoder(pAvCodecContext->codec_id);
		pAvCodecContext->debug_mv = 0;
		pAvCodecContext->debug = 0;
		pAvCodecContext->workaround_bugs = 1;
		pAvCodecContext->lowres = 0;
		pAvCodecContext->idct_algo= FF_IDCT_AUTO;
		pAvCodecContext->skip_frame= AVDISCARD_DEFAULT;
		pAvCodecContext->skip_idct= AVDISCARD_DEFAULT;
		pAvCodecContext->skip_loop_filter= AVDISCARD_DEFAULT;
		pAvCodecContext->error_recognition= FF_ER_CAREFUL;
		pAvCodecContext->error_concealment= 3;
		pAvCodecContext->thread_count= 1;
	
	//	set_context_opts(pAvCodecContext, avcodec_opts[pAvCodecContext->codec_type], 0, pAvCodec);
	    if ((pAvCodec == NULL) || (avcodec_open(pAvCodecContext, pAvCodec) < 0))
		{
			throw new CPlayerException("avcodec_open failed");
		}
	
	    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
	    switch(pAvCodecContext->codec_type)
		{
			case AVMEDIA_TYPE_AUDIO:
				if (pAvCodecContext->channels > 0) 
				{
					pAvCodecContext->request_channels = FFMIN(2, pAvCodecContext->channels);
				}
				else 
				{
					pAvCodecContext->request_channels = 2;
				}
				audio_src_fmt= pAvCodecContext->sample_fmt;
				audio_stream = stream_index;
				audio_st = ic->streams[stream_index];
				audio_buf_size = 0;
				audio_buf_index = 0;
				/* init averaging filter */
				audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
				audio_diff_avg_count = 0;
				/* since we do not have a precise anough audio fifo fullness,
				 we correct audio sync only if larger than this threshold */
				audio_diff_threshold = 2.0 * 1024 / pAvCodecContext->sample_rate;
				memset(&audio_pkt, 0, sizeof(audio_pkt));
				audioq.Init();
				break;
			
			case AVMEDIA_TYPE_VIDEO:
				video_stream = stream_index;
				video_st = ic->streams[stream_index];
				videoq.Init();
				break;
		
			default:
				break;
	    }
	}
    return 0;
}

/**
 * compute the exact PTS for the picture if it is omitted in the stream
 * @param pts1 the dts of the pkt / pts of the frame
 */
int CLocalPlayer::OutputPicture(AVFrame *src_frame, double pts1)
{
    double pts = pts1;
    if (pts != 0) 
	{
        /* update video clock with pts, if present */
        video_clock = pts;
    } 
	else
	{
        pts = video_clock;
    }
    /* update video clock for next frame */
	double frame_delay = (m_ePlayerType == ePlayerTypeStream) ? 1.0/25 : av_q2d(video_st->codec->time_base);	

    /* for MPEG2, the frame can be repeated, so we update the
	 clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    video_clock += frame_delay;
    return QueuePicture(src_frame, pts);
}

double CLocalPlayer::ComputeTargetTime(double frame_current_pts)
{
    double delay, sync_threshold, diff;
	
    /* compute nominal delay */
    delay = frame_current_pts - frame_last_pts;
    if (delay <= 0 || delay >= 10.0) {
        /* if incorrect delay, use previous one */
        delay = frame_last_delay;
    } else {
        frame_last_delay = delay;
    }
    frame_last_pts = frame_current_pts;
	
	if (m_ePlayerType == ePlayerTypeStream)
	{
		if (((av_sync_type == AV_SYNC_AUDIO_MASTER) ||
	         av_sync_type == AV_SYNC_EXTERNAL_CLOCK)) {
	        /* if video is slave, we try to correct big delays by
			 duplicating or deleting a frame */
	        diff = GetVideoClock() - GetMasterClock();
			//AddLog("v:%.3f   a:%.3f", GetVideoClock(), GetAudioClock());
	        /* skip or repeat frame. We take into account the
			 delay to compute the threshold. I still don't know
			 if it is the best guess */
	        sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay);
	        if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
	            if (diff <= -sync_threshold)
	                delay = 0;
	            else if (diff >= sync_threshold)
	                delay = 2 * delay;
	        }
	    }
	}
	else
	{
		 if (((av_sync_type == AV_SYNC_AUDIO_MASTER && audio_st) ||
	         av_sync_type == AV_SYNC_EXTERNAL_CLOCK)) {
	        /* if video is slave, we try to correct big delays by
			 duplicating or deleting a frame */
	        diff = GetVideoClock() - GetMasterClock();
		
	        /* skip or repeat frame. We take into account the
			 delay to compute the threshold. I still don't know
			 if it is the best guess */
	        sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay);
	        if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
	            if (diff <= -sync_threshold)
	                delay = 0;
	            else if (diff >= sync_threshold)
	                delay = 2 * delay;
	        }
	    }
	}
    /* update delay to follow master synchronisation source */
   
    frame_timer += delay;
#if defined(DEBUG_SYNC)
    printf("video: delay=%0.3f actual_delay=%0.3f pts=%0.3f A-V=%f\n",
		   delay, actual_delay, frame_current_pts, -diff);
#endif
	
    return frame_timer;
}

int CLocalPlayer::QueuePicture(AVFrame *src_frame, double pts1)
{
	/* wait until we have space to put a new picture */
    pthread_mutex_lock(m_pMutexPictQueue);
    while ((pictq_size >= VIDEO_PICTURE_QUEUE_SIZE)
		   && (!videoq.IsAbortRequested()))
	{
        pthread_cond_wait(m_pConditionPictQueue, m_pMutexPictQueue);
    }
    pthread_mutex_unlock(m_pMutexPictQueue);
	
    if (videoq.IsAbortRequested())
	{
        return -1;
	}
	{
		// update subtitle
		SSubTitleFormat sSubTitleFormat;
		wstring wstrSubTitle;
		m_srtSubTitleReader.GetString(pts1 * 1000, wstrSubTitle, &sSubTitleFormat);
		m_pVideoLocalPlayer->m_wstrSubTitle = wstrSubTitle;
	}	
	m_pVideoLocalPlayer->UpdateData(src_frame, pictq_windex);

	pts[pictq_windex] = pts1;
	targetTime[pictq_windex] = ComputeTargetTime(pts1);
	/* now we can update the picture count */
	if (++pictq_windex == VIDEO_PICTURE_QUEUE_SIZE)
	{
		pictq_windex = 0;
	}
	pthread_mutex_lock(m_pMutexPictQueue);
	pictq_size++;
	pthread_mutex_unlock(m_pMutexPictQueue);
	
    return 0;
}

/**
 *
 * @param pts the dts of the pkt / pts of the frame and guessed if not known
 */
//int CLocalPlayer::QueuePicture(AVFrame *src_frame, double pts1)
//{
//	double dMasterPts = GetMasterClock();
//	
//	double time= av_gettime()/1000000.0;	
//	if(time < dMasterPts)
//	{
//		usleep(dMasterPts - time);
//	}
//	/* update current video pts */
//	video_current_pts = pts1;
//	video_current_pts_drift = video_current_pts - time;
//	m_pVideoLocalPlayer->UpdateData(src_frame, 0);
//	
//	return 0;
//}


void* CLocalPlayer::ThreadRefresh(void* pParam)
{
	CLocalPlayer* pLocalPlayer = (CLocalPlayer*)pParam;
	if (pLocalPlayer == NULL)
	{
		return NULL;
	}
	return pLocalPlayer->ThreadRefreshRoute();
}

void* CLocalPlayer::ThreadRefreshRoute()
{
	while (!abort_request)
	{
		if ((m_ePlayerType == ePlayerTypeStream)
		|| ((m_ePlayerType != ePlayerTypeStream) && video_st)) 
		{
			//AddLog("VideoRefreshTimer\n");
			if (pictq_size == 0) 
			{
				usleep(10000);
				continue;
				//nothing to do, no picture to display in the que
			} 
			else
			{
				double time= av_gettime()/1000000.0;
				if (targetTime[pictq_rindex] > time)
				{
					usleep(1000);
					continue;
				}
				*m_pCurrentTime = GetMasterClock();
				
				/* update current video pts */
				video_current_pts = pts[pictq_rindex];
				video_current_pts_drift = video_current_pts - time;
				
				;
				// Compute delay
				/* display picture */
				VideoDisplay();
				
				/* update queue size and signal for next picture */
				if (++pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
				{
					pictq_rindex = 0;
				}
				
				pthread_mutex_lock(m_pMutexPictQueue);
				pictq_size--;
				pthread_cond_signal(m_pConditionPictQueue);
				pthread_mutex_unlock(m_pMutexPictQueue);
			}
		} 
		else if (audio_st)
		{
			VideoDisplay();
		}
	}
	return NULL;
}

void* CLocalPlayer::ThreadVideo(void* pParam)
{
	CLocalPlayer* pLocalPlayer = (CLocalPlayer*)pParam;
	if (pLocalPlayer == NULL)
	{
		return NULL;
	}
	return pLocalPlayer->ThreadVideoRoute();
}

void* CLocalPlayer::ThreadVideoRoute()
{
    AVPacket pkt1;
    int len1 = 0;
	int got_picture = 0;
    AVFrame *frame= avcodec_alloc_frame();
    double pts = 0.0;
	
    for(;;) 
	{
		while (paused && !videoq.IsAbortRequested())
		{
			usleep(10000);
		}
        if (videoq.Get(&pkt1, 1) < 0)
		{
            break;
		}
		
        if(pkt1.data == CPacketQueue::GetFlushAvPacket().data)
		{
			if (m_ePlayerType != ePlayerTypeStream)
			{
            	avcodec_flush_buffers(video_st->codec);
			}
			frame_last_pts = AV_NOPTS_VALUE;
			frame_last_delay = 0;
			frame_timer = (double)av_gettime() / 1000000.0;
            continue;
        }

        /* NOTE: ipts is the PTS of the _first_ picture beginning in
		 this packet, if any */
		if (m_ePlayerType == ePlayerTypeStream)
		{
        	len1 = avcodec_decode_video2(m_pCodecContextVideo,
									 frame, &got_picture,
									 &pkt1);
		}
		else
		{
			video_st->codec->reordered_opaque= pkt1.pts;
        	len1 = avcodec_decode_video2(video_st->codec,
									 frame, &got_picture,
									 &pkt1);
		}
		
        if((pkt1.dts == AV_NOPTS_VALUE) && (frame->reordered_opaque != AV_NOPTS_VALUE))
		{
            pts= frame->reordered_opaque;
		}
        else if(pkt1.dts != AV_NOPTS_VALUE)
		{
            pts= pkt1.dts;
		}
        else
		{
            pts= 0;
		}
		if (m_ePlayerType == ePlayerTypeStream)
		{
			pts /= 1000;
		}
		else
		{
       		pts *= av_q2d(video_st->time_base);
		}
		
        if (got_picture) 
		{
			if (OutputPicture(frame, pts) < 0)
			{
                goto the_end;
			}
        }
        av_free_packet(&pkt1);
    }
the_end:
    av_free(frame);
	//printf("\n---------------------------------------------Video thread end");
    return 0;
}


/* this thread gets the stream from the disk or the network */
void* CLocalPlayer::ThreadRead(void* pParam)
{
	CLocalPlayer* pLocalPlayer = (CLocalPlayer*)pParam;
	if (pLocalPlayer == NULL)
	{
		return NULL;
	}
	return pLocalPlayer->ThreadReadRoute();
}

void* CLocalPlayer::ThreadReadRoute()
{
	{
		int ret = 0;
		int eof = 0;
		AVPacket pkt = {0};
	    for( ; ;)
		{
	        if (abort_request)
			{
	            break;
			}
	        if (paused != last_paused)
			{
	            last_paused = paused;
	            if (paused)
				{
	                read_pause_return= av_read_pause(ic);
				}
	            else
				{
	                av_read_play(ic);
				}
	        }
		
	        if (seek_req) 
			{
				ret = avformat_seek_file(ic, 
										 (video_st == NULL) ? audio_st->index : video_st->index, 
										 seek_pos, 
										 seek_pos, 
										 seek_pos, 
										 seek_flags);
	            if (ret < 0) 
				{
	                fprintf(stderr, "%s: error while seeking\n", ic->filename);
	            }
				else
				{
	                if (audio_stream >= 0) 
					{
						audioq.Flush();
						audioq.Put(const_cast<AVPacket*>(&CPacketQueue::GetFlushAvPacket()));
	                }
	                if (video_stream >= 0)
					{
						videoq.Flush();
						videoq.Put(const_cast<AVPacket*>(&CPacketQueue::GetFlushAvPacket()));
	                }
	            }
	            seek_req = 0;
	            eof= 0;
	        }
		
	        /* if the queue are full, no need to read more */
	        if ( (audioq.GetSize() + videoq.GetSize() > MAX_QUEUE_SIZE)
	            || (   (audioq.GetSize()  > MIN_AUDIOQ_SIZE || audio_stream<0)
	                && (videoq.GetNbPackets() > MIN_FRAMES || video_stream<0)))
			{
				/* wait 10 ms */
				//            SDL_Delay(10);
	            usleep(100000);
	           // AddLog("Read too much\n");
				continue;
			}
	        if(eof)
			{
	            if(video_stream >= 0)
				{
	                av_init_packet(&pkt);
	                pkt.data=NULL;
	                pkt.size=0;
	                pkt.stream_index= video_stream;
					videoq.Put(&pkt);
	            }
				usleep(10000);
	            if(audioq.GetSize() + videoq.GetSize() ==0)
				{
					*m_pIsReadEndOfFile = true;
					break;
	            }
	            continue;
	        }
	        ret = av_read_frame(ic, &pkt);
        
	        if (ret < 0) 
			{
	            if (ret == AVERROR_EOF || url_feof(ic->pb))
				{
	                eof=1;
				}
				if (url_ferror(ic->pb))
				{
					break;
				}
				usleep(10000);
				continue;
	        }
	        /* check if packet is in play range specified by user, then queue, otherwise discard */
	        if (pkt.stream_index == audio_stream) 
			{
				audioq.Put(&pkt);
	        }
			else if (pkt.stream_index == video_stream) 
			{
				videoq.Put(&pkt);
	        } 
			else 
			{
	            av_free_packet(&pkt);
	        }
	    }
	    AddLog("read wait until end\n");
	    /* wait until the end */
	
		while (!abort_request) 
		{
			usleep(100000);
		}
	
		ret = 0;
		// fail:
	    /* disable interrupting */
	
	    /* close each stream */
		if (audio_stream >= 0)
		{
	        StreamComponentClose(audio_stream);
		}
	    if (video_stream >= 0)
		{
	        StreamComponentClose(video_stream);
		}
    
	    if (ic) 
		{
	        av_close_input_file(ic);
	        ic = NULL; /* safety */
	    }
	    url_set_interrupt_cb(NULL);
		return NULL;
	}
}



void CLocalPlayer::VideoImageDisplay()
{
	m_pVideoLocalPlayer->Show(pictq_rindex);    
}



/* display the current picture, if any */
void CLocalPlayer::VideoDisplay()
{
	if (m_ePlayerType == ePlayerTypeStream)
	{
		VideoImageDisplay();
	}
	else
	{
	    if (video_st)
		{
			VideoImageDisplay();
		}
		else if (audio_st)
		{
			VideoAudioDisplay();
		}
	}
}


/* get the current audio clock value */
double CLocalPlayer::GetAudioClock()
{
    double pts;
    int hw_buf_size, bytes_per_sec;
    pts = audio_clock;
    hw_buf_size = AudioWriteGetBufSize();
    bytes_per_sec = 0;
	if (m_ePlayerType == ePlayerTypeStream)
	{
			 bytes_per_sec = 44100 * 2 * 2;
	}
	else
	{
	    if (audio_st) 
		{
	        bytes_per_sec = audio_st->codec->sample_rate *
			2 * audio_st->codec->channels;
	    }
	}
    if (bytes_per_sec)
	{
        pts -= (double)hw_buf_size / bytes_per_sec;
    }
	return pts;
}

/* get the current video clock value */
double CLocalPlayer::GetVideoClock()
{
    if (paused)
	{
        return video_current_pts;
    }
	else
	{
        return video_current_pts_drift + av_gettime() / 1000000.0;
    }
}

/* get the current external clock value */
double CLocalPlayer::GetExternalClock()
{
    int64_t ti;
    ti = av_gettime();
    return external_clock + ((ti - external_clock_time) * 1e-6);
}

/* get the current master clock value */
double CLocalPlayer::GetMasterClock()
{
	if (m_ePlayerType == ePlayerTypeStream)
	{
		return GetAudioClock();
	}
	else
	{
	    double val;
	
	    if (av_sync_type == AV_SYNC_VIDEO_MASTER)
		{
	        if (video_st)
			{
	            val = GetVideoClock();
	        }
			else
	        {
				val = GetAudioClock();
			}
	    } 
		else if (av_sync_type == AV_SYNC_AUDIO_MASTER) 
		{
	        if (audio_st)
	        {
				val = GetAudioClock();
			}
	        else
			{
				val = GetVideoClock();
			}
	    } 
		else
		{
	        val = GetExternalClock();
	    }
	    return val;
	}
}

/* seek in the stream */
void CLocalPlayer::StreamSeek(int64_t pos)
{
	if (m_ePlayerType == ePlayerTypeStream)
	{
		if (!seek_req)
		{
			*m_pInNetSeeking = 1;
	        seek_pos = pos;
			seek_flags = AVSEEK_FLAG_FRAME;
       
			// clear old data
			audioq.Flush();
			audioq.Put(const_cast<AVPacket*>(&CPacketQueue::GetFlushAvPacket()));
		
			videoq.Flush();
			videoq.Put(const_cast<AVPacket*>(&CPacketQueue::GetFlushAvPacket()));
			seek_req = 1;
		}
	}
	else
	{	
	    if (!seek_req)
		{
	        seek_pos = pos;
			seek_flags = AVSEEK_FLAG_FRAME;
	        seek_req = 1;
	    }
	}
}

/* pause or resume the video */
void CLocalPlayer::StreamPause()
{
    if (paused) 
	{
        frame_timer += av_gettime() / 1000000.0 
									+ video_current_pts_drift - video_current_pts;
        if(read_pause_return != AVERROR(ENOSYS)){
            video_current_pts = video_current_pts_drift + av_gettime() / 1000000.0;
        }
        video_current_pts_drift = video_current_pts - av_gettime() / 1000000.0;
    }
    paused = !paused;
}

void CLocalPlayer::VideoAudioDisplay()
{
	// TODO
}


/* return the new audio buffer size (samples can be added or deleted
 to get better sync if video or external master clock) */
int CLocalPlayer::SynchronizeAudio(short *samples, int samples_size1, double pts)
{
    int n, samples_size;
    double ref_clock;
	
	if (m_ePlayerType == ePlayerTypeStream)
	{
		n = 2 * 2;
	}
	else
	{
    	n = 2 * audio_st->codec->channels;
    }
	samples_size = samples_size1;
	
    /* if not master, then we try to remove or add samples to correct the clock */
	bool bHasVideo = (m_ePlayerType == ePlayerTypeStream) ? true : (video_st != NULL);
    if (((av_sync_type == AV_SYNC_VIDEO_MASTER && bHasVideo)
		 || av_sync_type == AV_SYNC_EXTERNAL_CLOCK))
	{
        double diff, avg_diff;
        int wanted_size, min_size, max_size, nb_samples;
		
        ref_clock = GetMasterClock();
        diff = GetAudioClock() - ref_clock;
		
        if (diff < AV_NOSYNC_THRESHOLD)
		{
            audio_diff_cum = diff + audio_diff_avg_coef * audio_diff_cum;
            if (audio_diff_avg_count < AUDIO_DIFF_AVG_NB)
			{
                /* not enough measures to have a correct estimate */
                audio_diff_avg_count++;
            } 
			else 
			{
                /* estimate the A-V difference */
                avg_diff = audio_diff_cum * (1.0 - audio_diff_avg_coef);
				
                if (fabs(avg_diff) >= audio_diff_threshold) 
				{
					if (m_ePlayerType == ePlayerTypeStream)
					{
						wanted_size = samples_size + ((int)(diff * 44100) * n);
                    }
					else
					{
						wanted_size = samples_size + ((int)(diff * audio_st->codec->sample_rate) * n);
					}
					nb_samples = samples_size / n;
					
                    min_size = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
                    max_size = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
                    if (wanted_size < min_size)
					{
                        wanted_size = min_size;
					}
                    else if (wanted_size > max_size)
					{
						wanted_size = max_size;
					}
					
                    /* add or remove samples to correction the synchro */
                    if (wanted_size < samples_size)
					{
                        /* remove samples */
                        samples_size = wanted_size;
                    } 
					else if (wanted_size > samples_size)
					{
                        uint8_t *samples_end, *q;
                        int nb;
						
                        /* add samples */
                        nb = (samples_size - wanted_size);
                        samples_end = (uint8_t *)samples + samples_size - n;
                        q = samples_end + n;
                        while (nb > 0)
						{
                            memcpy(q, samples_end, n);
                            q += n;
                            nb -= n;
                        }
                        samples_size = wanted_size;
                    }
                }
				
            }
        } 
		else 
		{
            /* too big difference : may be initial PTS errors, so
			 reset A-V filter */
            audio_diff_avg_count = 0;
            audio_diff_cum = 0;
        }
    }
	
    return samples_size;
}

/* decode one audio frame and returns its uncompressed size */
int CLocalPlayer::AudioDecodeFrame(double *pts_ptr)
{
    AVPacket *pkt_temp = &audio_pkt_temp;
    AVPacket *pkt = &audio_pkt;
    AVCodecContext *dec= (m_ePlayerType == ePlayerTypeStream) ? m_pCodecContextAudio : audio_st->codec;
    int n, len1, data_size;
    double pts;
	
    for(;;)
	{
        /* NOTE: the audio packet can contain several frames */
        while (pkt_temp->size > 0)
		{
            data_size = sizeof(audio_buf1);
            len1 = avcodec_decode_audio3(dec,
										 (int16_t *)audio_buf1, &data_size,
										 pkt_temp);
            if (len1 < 0)
			{
                /* if error, we skip the frame */
                pkt_temp->size = 0;
                break;
            }
			
            pkt_temp->data += len1;
            pkt_temp->size -= len1;
            if (data_size <= 0)
			{
                continue;
			}
			{
				if (dec->sample_fmt != audio_src_fmt)
				{
					if (reformat_ctx)
					{
						av_audio_convert_free(reformat_ctx);
					}
					reformat_ctx= av_audio_convert_alloc(SAMPLE_FMT_S16, 1,
															 dec->sample_fmt, 1, NULL, 0);
					if (!reformat_ctx) 
					{
						printf("Cannot convert auido sample format \n");
						break;
					}
					audio_src_fmt= dec->sample_fmt;
				}

				if (reformat_ctx) 
				{
					const void *ibuf[6]= {audio_buf1};
					void *obuf[6]= {audio_buf2};
					int istride[6]= {av_get_bits_per_sample_fmt((AVSampleFormat)dec->sample_fmt)/8};
					int ostride[6]= {2};
					int len= data_size/istride[0];
					if (av_audio_convert(reformat_ctx, obuf, ostride, ibuf, istride, len)<0) 
					{
						printf("av_audio_convert() failed\n");
					   break;
				   }
				}
			}
			audio_buf= audio_buf2;
			/* FIXME: existing code assume that data_size equals framesize*channels*2
			 remove this legacy cruft */
			//                data_size= len*2;
			//            }else{
			audio_buf= audio_buf1;
			//           }
			
            /* if no pts, then compute it */
            pts = audio_clock;
            *pts_ptr = pts;
            n = 2 * dec->channels;
            audio_clock += (double)data_size / (double)(n * dec->sample_rate);
			
			
            return data_size;
        }
		
        /* free the current packet */
		if (pkt->data)
		{
			av_free_packet(pkt);
		}
        if (paused || audioq.IsAbortRequested())
		{
            return -1;
        }
		
        /* read next packet */
        if (audioq.Get(pkt, 1) < 0)
		{
            return -1;
		}
        if(pkt->data == CPacketQueue::GetFlushAvPacket().data)
		{
            avcodec_flush_buffers(dec);
            continue;
        }
		
        pkt_temp->data = pkt->data;
        pkt_temp->size = pkt->size;
		
        /* if update the audio clock with the pts */
		if (pkt->pts != AV_NOPTS_VALUE) 
		{
			if (m_ePlayerType == ePlayerTypeStream)
			{
				audio_clock = pkt->pts / 1000.0;
			}
			else
			{
				audio_clock = av_q2d(audio_st->time_base)*pkt->pts;
			}
		}
    }
}

/* get the current audio output buffer size, in samples. With SDL, we
 cannot have a precise information */
int CLocalPlayer::AudioWriteGetBufSize()
{
    return audio_buf_size - audio_buf_index;
}




void CLocalPlayer::StreamComponentClose(int stream_index)
{
	{
		if (stream_index < 0 || stream_index >= ic->nb_streams)
		{
	        return;
		}

	    AVCodecContext* avctx = ic->streams[stream_index]->codec;
		switch(avctx->codec_type) 
		{
			case AVMEDIA_TYPE_AUDIO:
				audioq.Abort();
				audioq.End();
				if (reformat_ctx)
				{
					av_audio_convert_free(reformat_ctx);
				}
				reformat_ctx = NULL;
				break;
			
			case AVMEDIA_TYPE_VIDEO:
				videoq.Abort();
				/* note: we also signal this mutex to make sure we deblock the
				 video thread in all cases */
	            pthread_mutex_lock(m_pMutexPictQueue);
	            pthread_cond_signal(m_pConditionPictQueue);
	            pthread_mutex_unlock(m_pMutexPictQueue);

				//printf("\n---------------------------------------------Read Wait Video");
				pthread_join(video_tid, NULL);
				videoq.End();
				break;
			
			default:
				break;
	    }
	
	    ic->streams[stream_index]->discard = AVDISCARD_ALL;
	    avcodec_close(avctx);
	    switch(avctx->codec_type)
		{
			case AVMEDIA_TYPE_AUDIO:
				audio_st = NULL;
				audio_stream = -1;
				break;
			
			case AVMEDIA_TYPE_VIDEO:
				video_st = NULL;
				video_stream = -1;
				break;

			default:
				break;
	    }
	}
}


inline int CLocalPlayer::ComputeMod(int a, int b)
{
    a = a % b;
    if (a >= 0)
	{
        return a;
	}
    else
	{
        return a + b;
	}
}


void CLocalPlayer::Step()
{	
	/* if the stream is paused unpause it, then step */
	if (paused)
	{
		StreamPause();
    }
	step = 1;
}

void CLocalPlayer::AudioCallBack(unsigned char *stream, int len)
{
    audio_callback_time = av_gettime();
	
    while (len > 0) 
	{
        if (audio_buf_index >= audio_buf_size)
		{
			double pts = 0.0;
			int audio_size = AudioDecodeFrame(&pts);
			if (audio_size < 0)
			{
                /* if error, just output silence */
				audio_buf = audio_buf1;
				audio_buf_size = 1024;
				memset(audio_buf, 0, audio_buf_size);
			}
			else 
			{
				audio_size = SynchronizeAudio((int16_t *)audio_buf, 
											  audio_size,
											  pts);
				audio_buf_size = audio_size;
			}
			audio_buf_index = 0;
        }
        int len1 = audio_buf_size - audio_buf_index;
        if (len1 > len)
		{
            len1 = len;
		}
        memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
	*m_pCurrentTime = GetMasterClock();
}

void CLocalPlayer::SeekBySecond(int iSecond)
{
	if (m_ePlayerType == ePlayerTypeStream)
	{
		StreamSeek(iSecond);
	}
	else
	{
		AVRational timeBase = {1, 1};
		if (video_st != NULL)
		{
			timeBase = video_st->time_base;
		}
		else if (audio_st != NULL)
		{
			timeBase = audio_st->time_base;
		}
		else
		{
			printf("Stream Seek but no stream found");	
			return;
		}

		seek_pos = (int64_t)((double)timeBase.den / timeBase.num * iSecond);
		StreamSeek(seek_pos);
	}
}


void CLocalPlayer::SetVolume(int iVolume)
{
	if (m_pAudioLocalPlayer == NULL)
	{
		printf("SetVolume but m_pAudioLocalPlayer == NULL");
		return;
	}
	m_pAudioLocalPlayer->SetVolume(iVolume);
}



void AudioCallback(void *opaque, unsigned char *stream, int len)
{
	CLocalPlayer* pLocalPlayer = (CLocalPlayer*)opaque;
	if (pLocalPlayer != NULL)
	{
		pLocalPlayer->AudioCallBack(stream, len);
	}
}


pthread_t StartThread(thread_func_t func, void *param)
{
	pthread_t pthread;
    pthread_attr_t attr;
    int retval;
    
    (void) pthread_attr_init(&attr);
    //(void) pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // TODO(lsm): figure out why mongoose dies on Linux if next line is enabled
    // (void) pthread_attr_setstacksize(&attr, sizeof(struct mg_connection) * 5);
    
    if ((retval = pthread_create(&pthread, &attr, func, param)) != 0) 
    {
        AddLog("pthread_create return error");
    }
    
    return pthread;
}

bool RectStretchAdapt(const CRect* pInnerRect, const CRect* pOuterRect, CRect* pAdaptRect)
{
    if (pInnerRect==NULL || pOuterRect==NULL || pAdaptRect==NULL)
    {
        return false;
    }
    if (IsRectEmpty(pInnerRect) || IsRectEmpty(pOuterRect))
    {
        return false;
    }
	
    double fInnerRate = pInnerRect->Width() / (double)pInnerRect->Height();
    double fOuterRate = pOuterRect->Width() / (double)pOuterRect->Height();
    *pAdaptRect = *pOuterRect;
	
    if (fInnerRate < fOuterRate)
    {
        int iAdaptRectWidth = pInnerRect->Width() * pOuterRect->Height()/(double)pInnerRect->Height();
        pAdaptRect->left = (pOuterRect->Width() - iAdaptRectWidth) / 2 + pOuterRect->left;
        pAdaptRect->right = pAdaptRect->left + iAdaptRectWidth;
		
    }
    else
    {
        int iAdaptRectHeight = pInnerRect->Height() * pOuterRect->Width()/(double)pInnerRect->Width();
        pAdaptRect->top = (pOuterRect->Height() - iAdaptRectHeight) / 2 + pOuterRect->top;
        pAdaptRect->bottom = pAdaptRect->top + iAdaptRectHeight;
    }
    return true;
}

bool IsRectEmpty(const CRect* pRect)
{
	if (pRect == NULL)
	{
		return true;
	}
	if ((pRect->right <= pRect->left) || (pRect->bottom <= pRect->top))
	{
		return true;
	}
	return false;
}

const char* FormatTime(unsigned long iSecond, bool bPositive, char* pszTime, int iLen)
{
	*pszTime = '\0';
	if (iLen < 20)
	{
		return pszTime;
	}
	int iHour = iSecond / 3600;
	int iMinute = (iSecond - iHour * 3600) / 60;
	iSecond = iSecond % 60;
	char szFlag[2] = {0};
	if (!bPositive)
	{
		szFlag[0] = '-';
	}
	if (iHour > 0)
	{
		sprintf(pszTime, "%s%02d:%02d:%02d", szFlag, iHour, iMinute, (int)iSecond);
	}
	else
	{	
		sprintf(pszTime, "%s%02d:%02d", szFlag, iMinute, (int)iSecond);
	}
	return pszTime;
}

void AssignRect(const CRect& rectFrom, CRect& rectTo)
{
    rectTo.left = rectFrom.left;
    rectTo.right = rectFrom.right;
    rectTo.top = rectFrom.top;
    rectTo.bottom = rectFrom.bottom;
}


bool CLocalPlayer::IsPaused()
{
	return paused;
}