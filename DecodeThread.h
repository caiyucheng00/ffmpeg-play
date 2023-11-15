#pragma once
#ifndef __DECODETHREAD__H_
#define __DECODETHREAD__H_

#include "Thread.h"
#include "AVPacketQueue.h"
#include "AVFrameQueue.h"

class DecodeThread : public Thread
{
public:
	DecodeThread(AVPacketQueue* packet_queue, AVFrameQueue* frame_queue, const char* name);
	~DecodeThread();

	int Init(AVCodecParameters* par);
	int Start();
	int Stop();
	void Run();
	AVCodecContext* getAVCodecContext();

private:
	AVCodecContext* codec_ctx = NULL;
	char err2str[256] = { 0 };
	const char* name_ = nullptr;
	AVPacketQueue* packet_queue_ = NULL;
	AVFrameQueue* frame_queue_ = NULL;
};


#endif
