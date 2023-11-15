#pragma once
#ifndef __AVFRAMEQUEUE__H_
#define __AVFRAMEQUEUE__H_

#include "Queue.h"

#ifdef __cplusplus
extern "C" {
#include "libavcodec/avcodec.h"
}
#endif

class AVFrameQueue
{
public:
	AVFrameQueue();
	~AVFrameQueue();

	void Abort();
	int Push(AVFrame* val);
	AVFrame* Pop(const int timeout);
	AVFrame* Front();
	int Size();

private:
	void release();
	Queue<AVFrame*> queue_;
};

#endif