#pragma once
#ifndef __DEMUXTHREAD__H_
#define __DEMUXTHREAD__H_

#include <iostream>
#include "Thread.h"
#include "AVPacketQueue.h"
#include "timeutils.h"

#ifdef __cplusplus
extern "C" {
    #include <libavutil/avutil.h>
    #include <libavformat/avformat.h>
}
#endif


class DemuxThread : public Thread
{
public:
	DemuxThread(AVPacketQueue* audio_packet_queue, AVPacketQueue* video_packet_queue);
	virtual ~DemuxThread();

	int Init(const char* url);
	virtual int Start();
	virtual int Stop();
	virtual void Run();

	AVCodecParameters* AudioCodecParameters();
	AVCodecParameters* VideoCodecParameters();

	AVRational AudioStreamTimebase();
	AVRational VideoStreamTimebase();

	bool IsTimeout();
	void ResetTime();
	int GetBlockTime();

private:
	static int decode_interrupt_cb(void* arg);

	std::string url_;
	AVFormatContext* infmt_ctx = NULL;
	int video_stream_idx_ = -1;
	int audio_stream_idx_ = -1;
	char err2str[256] = { 0 }; // error对应字符串
	AVPacketQueue* audio_packet_queue_ = NULL;
	AVPacketQueue* video_packet_queue_ = NULL;

	// 处理超时
	int64_t time_out_ = 3000;
	int64_t pre_time_ = 0;   // 记录调用前
};


#endif

