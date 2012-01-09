/*
 *  QueuePacket.h
 *  FFmpegTest
 *
 *  Create by xiaoyi liao  on 11-2-25.
 *  Copyright 2011 . All rights reserved.
 *
 */
#ifndef _PACKET_QUEUE_H_
#define _PAKCET_QUEUE_H_

#include "pthread.h"
#include "Common.h"

#define MAX_PACKET_COUNT 100

class CPacketQueueAssist;

class CPacketQueue
{
public:
    CPacketQueue();
	void Init();
    ~CPacketQueue();
	void Flush();
	void End();
	int Put(AVPacket *pkt);
	void Abort();
	/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
	int Get(AVPacket *pkt, int block);
	bool IsAbortRequested();
	static const AVPacket& GetFlushAvPacket();
	
	int GetNbPackets();
	int GetSize();
private:
	
	AVPacketList *first_pkt;
	AVPacketList *last_pkt;
    int nb_packets;
    int size;
    int abort_request;
    pthread_mutex_t m_mutex;
	pthread_cond_t m_condition;
	
	static AVPacket flush_pkt;
	friend class CPacketQueueAssist;
};

class CPacketQueueAssist
{
public:
	static void Combine(CPacketQueue& packetTo, CPacketQueue& packetFrom);
};

//class CPacketQueue2
//{
//public:
//    CPacketQueue2();
//	void Init();
//    ~CPacketQueue2();
//	void Flush();
//	void End();
//	int Put(AVPacket *pkt);
//	void Abort();
//	/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
//	int Get(AVPacket *pkt, int block);
//	bool IsAbortRequested();
//	static const AVPacket& GetFlushAvPacket();
//	
//	int GetNbPackets();
//	int GetSize();
//private:
//	AVPacket m_avPacket1[MAX_PACKET_COUNT];
//	int m_iPacket1Count;
//	AVPacket m_avPacket2[MAX_PACKET_COUNT];
//	int m_iPacket2Count;
//	
//	static AVPacket flush_pkt;
//};


#endif