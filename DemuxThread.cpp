#include "DemuxThread.h"

DemuxThread::DemuxThread(AVPacketQueue* audio_packet_queue, AVPacketQueue* video_packet_queue) :
	audio_packet_queue_(audio_packet_queue),
	video_packet_queue_(video_packet_queue)
{
	printf("DemuxThread\n");
}

DemuxThread::~DemuxThread()
{
	printf("~DemuxThread\n");
}

int DemuxThread::Init(const char* url)
{
	int ret = 0;

	//1.判断url 和队列 
	if (!url) {
		printf("%s(%d) url is null: \n", __FUNCTION__, __LINE__);
		return -1;
	}
	url_ = url;

	if (!audio_packet_queue_ || !video_packet_queue_) {
		printf("%s(%d) audio_packet_queue or video_packet_queue is null: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	//2.分配解复用器上下文
	infmt_ctx = avformat_alloc_context();
	ResetTime();
	ret = avformat_open_input(&infmt_ctx, url_.c_str(), NULL, NULL);
	if (ret < 0) {
		av_strerror(ret, err2str, sizeof(err2str));
		printf("%s(%d) avformat_open_input failed: %d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
		return -1;
	}

	ret = avformat_find_stream_info(infmt_ctx, NULL);   // 音频视频对应编码格式
	if (ret < 0) {
		av_strerror(ret, err2str, sizeof(err2str));
		printf("%s(%d) avformat_find_stream_info failed: %d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
		return -1;
	}

	av_dump_format(infmt_ctx, 0, url_.c_str(), 0); //打印相关信息

	video_stream_idx_ = av_find_best_stream(infmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	audio_stream_idx_ = av_find_best_stream(infmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	printf("%s(%d) video_stream_idx:%d, audio_stream_idx:%d\n", __FUNCTION__, __LINE__, video_stream_idx_, audio_stream_idx_);
	if (video_stream_idx_ < 0 || audio_stream_idx_ < 0) {
		printf("no audio or no video\n");
		return -1;
	}

	return 0;
}

int DemuxThread::Start()
{
	printf("%s(%d)\n", __FUNCTION__, __LINE__);
	// 开启线程
	thread_ = new std::thread(&DemuxThread::Run, this);
	if (!thread_) {
		printf("%s(%d) new DemuxThread failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}
	return 0;
}

int DemuxThread::Stop()
{
	printf("%s(%d)\n", __FUNCTION__, __LINE__);
	avformat_close_input(&infmt_ctx);
	Thread::Stop();
	return 0;
}

void DemuxThread::Run()
{
	printf("DemuxThread::Run() into\n");
	AVPacket* packet = av_packet_alloc();
	int ret = 0;

	while (1) {
		if (abort_ == 1) {   // 线程退出标志位==1
			break;
		}

		// 队列长度控制
		if (audio_packet_queue_->Size() > 100 || video_packet_queue_->Size() > 100) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		ret = av_read_frame(infmt_ctx, packet);
		if (ret == AVERROR_EOF) {
			av_strerror(ret, err2str, sizeof(err2str));
			printf("%s(%d) av_read_frame failed: %d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
			printf("av_read_frame OVER\n");
			break;
		}

		//视频包队列 索引0
		if (packet->stream_index == video_stream_idx_) {
			video_packet_queue_->Push(packet); 
			//printf("video packet size: %d\n", video_packet_queue_->Size());
		}
		//音频包队列 索引1
		else if (packet->stream_index == audio_stream_idx_) {
			audio_packet_queue_->Push(packet);
			//printf("audio packet size: %d\n", audio_packet_queue_->Size());
		}
		// 释放
		else {
			av_packet_unref(packet);
		}
	}

	//跳出关闭
	printf("DemuxThread::Run() leave\n");
}

AVCodecParameters* DemuxThread::AudioCodecParameters()
{
	if (audio_stream_idx_ != -1) {
		return infmt_ctx->streams[audio_stream_idx_]->codecpar;
	}

	return NULL;
}

AVCodecParameters* DemuxThread::VideoCodecParameters()
{
	if (video_stream_idx_ != -1) {
		return infmt_ctx->streams[video_stream_idx_]->codecpar;
	}

	return NULL;
}

AVRational DemuxThread::AudioStreamTimebase()
{
	if (audio_stream_idx_ != -1) {
		return infmt_ctx->streams[audio_stream_idx_]->time_base;
	}

	return AVRational{0,1};
}

AVRational DemuxThread::VideoStreamTimebase()
{
	if (video_stream_idx_ != -1) {
		return infmt_ctx->streams[video_stream_idx_]->time_base;
	}

	return AVRational{ 0,1 };
}

bool DemuxThread::IsTimeout()
{
	if (TimesUtil::GetTimeMillisecond() - pre_time_ > time_out_) {
		return true;
	}

	return false;
}

void DemuxThread::ResetTime()
{
	pre_time_ = TimesUtil::GetTimeMillisecond();   // 重置
}

int DemuxThread::GetBlockTime()
{
	return TimesUtil::GetTimeMillisecond() - pre_time_;
}
