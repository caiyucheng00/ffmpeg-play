#pragma once
#ifndef __AVPACKETQUEUE__H_
#define __AVPACKETQUEUE__H_

#include "Queue.h"

#ifdef __cplusplus
extern "C" {
    #include <libavcodec/avcodec.h>
}
#endif

class AVPacketQueue
{
public:
	AVPacketQueue();
	~AVPacketQueue();

	void Abort();
	int Push(AVPacket* val);
	AVPacket* Pop(const int timeout);
	int Size();

private:
	void release();
	Queue<AVPacket*> queue_;
};

#endif