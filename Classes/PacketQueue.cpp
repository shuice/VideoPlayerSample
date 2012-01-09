/*
 *  QueuePacket.cpp
 *  FFmpegTest
 *
 *  Create by xiaoyi liao  on 11-2-25.
 *  Copyright 2011 . All rights reserved.
 *
 */

#include "PacketQueue.h"
#include "PlayerException.h"




AVPacket CPacketQueue::flush_pkt = {0};
//AVPacket CPacketQueue2::flush_pkt = {0};
/* packet queue handling */
CPacketQueue::CPacketQueue()
{
 	if (flush_pkt.data == NULL)
	{
		av_init_packet(&flush_pkt);
		flush_pkt.data = (uint8_t*)"FLUSH";
	}
	first_pkt = NULL;
	last_pkt = NULL;
    nb_packets = 0;
    size = 0;
    abort_request = 0;
}

void CPacketQueue::Init()
{
	if (flush_pkt.data == NULL)
	{
		av_init_packet(&flush_pkt);
		flush_pkt.data = (uint8_t*)"FLUSH";
	}
	first_pkt = NULL;
	last_pkt = NULL;
    nb_packets = 0;
    size = 0;
    abort_request = 0;

    pthread_cond_init(&m_condition, NULL);
    pthread_mutex_init(&m_mutex, NULL);
	
    Put(&flush_pkt);
}

CPacketQueue::~CPacketQueue()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_condition);
    
}

void CPacketQueue::Flush()
{
    AVPacketList *pkt = NULL;
	AVPacketList* pkt1 = NULL;
	
    pthread_mutex_lock(&m_mutex);
    for(pkt = first_pkt; pkt != NULL; pkt = pkt1) 
	{
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    last_pkt = NULL;
    first_pkt = NULL;
    nb_packets = 0;
    size = 0;
    pthread_mutex_unlock(&m_mutex);
}

void CPacketQueue::End()
{
    Flush();
}

int CPacketQueue::Put(AVPacket *pkt)
{
    AVPacketList *pkt1 = NULL;
	
    /* duplicate the packet */
    if (pkt!=&flush_pkt && av_dup_packet(pkt) < 0)
	{
        return -1;
	}
	
    pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
    if (!pkt1)
	{
        return -1;
	}
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
	
	pthread_mutex_lock(&m_mutex);
    if (last_pkt == 0)
	{		
        first_pkt = pkt1; 
    }
	else
	{
        last_pkt->next = pkt1;
    }
	last_pkt = pkt1;
    nb_packets++;
    size += pkt1->pkt.size + sizeof(*pkt1);
    pthread_cond_signal(&m_condition);
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

void CPacketQueue::Abort()
{
    pthread_mutex_lock(&m_mutex);
    abort_request = 1;
	pthread_cond_signal(&m_condition);
    pthread_mutex_unlock(&m_mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int CPacketQueue::Get(AVPacket *pkt, int block)
{
    AVPacketList* pkt1 = NULL;
    int iRet = 0;
    pthread_mutex_lock(&m_mutex);
    for(;;) 
	{
        if (abort_request)
		{
            iRet = -1;
            break;
        }
		
        pkt1 = first_pkt;
        if (pkt1)
        {
            first_pkt = pkt1->next;
            if (first_pkt == 0)
			{
                last_pkt = NULL;
			}
            nb_packets--;
            size -= pkt1->pkt.size + sizeof(*pkt1);
            *pkt = pkt1->pkt;
            av_free(pkt1);
            iRet = 1;
            break;
        } 
		else if (block == 0)
		{
            iRet = 0;
            break;
        } 
		else
		{
            pthread_cond_wait(&m_condition, &m_mutex);
        }
    }
    pthread_mutex_unlock(&m_mutex);
    return iRet;
}

const AVPacket& CPacketQueue::GetFlushAvPacket()
{
	return flush_pkt;
}

bool CPacketQueue::IsAbortRequested()
{
	return abort_request;
}

int CPacketQueue::GetNbPackets()
{
	return nb_packets;
}

int CPacketQueue::GetSize()
{
	return size;
}

void CPacketQueueAssist::Combine(CPacketQueue& packetTo, CPacketQueue& packetFrom)
{
	pthread_mutex_lock(&packetTo.m_mutex);
	if (packetFrom.first_pkt == NULL)
	{
		pthread_mutex_unlock(&packetTo.m_mutex);
		return;
	}
	if (packetTo.first_pkt == NULL)
	{
		packetTo.first_pkt = packetFrom.first_pkt;
	}
	else
	{
		packetTo.last_pkt->next = packetFrom.first_pkt;
	}
	packetTo.last_pkt = packetFrom.last_pkt;
	packetTo.nb_packets += packetFrom.nb_packets;
	packetTo.size		+= packetFrom.size;
	pthread_cond_signal(&packetTo.m_condition);
	pthread_mutex_unlock(&packetTo.m_mutex);
	
	packetFrom.first_pkt = NULL;
	packetFrom.last_pkt = NULL;
	packetFrom.nb_packets = 0;
	packetFrom.size = 0;
}

#pragma mark -------