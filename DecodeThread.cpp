#include "DecodeThread.h"

DecodeThread::DecodeThread(AVPacketQueue* packet_queue, AVFrameQueue* frame_queue, const char* name) :
	packet_queue_(packet_queue),
	frame_queue_(frame_queue),
	name_(name)
{
	printf("%s: p%d f%d\n", name_, packet_queue_->Size(), frame_queue_->Size());
}

DecodeThread::~DecodeThread()
{
	printf("%s: ~DecodeThread\n", name_);
}

int DecodeThread::Init(AVCodecParameters* par)
{
	int ret = 0;
	//判断
	if (!par) {
		printf("%s(%d) AVCodecParameters is null: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	//1.创建codec_ctx
	codec_ctx = avcodec_alloc_context3(NULL);
	ret = avcodec_parameters_to_context(codec_ctx, par);
	if (ret < 0) {
		av_strerror(ret, err2str, sizeof(err2str));
		printf("%s(%d) avcodec_parameters_to_context failed: %d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
		return -1;
	}

	const AVCodec* codec = avcodec_find_decoder(codec_ctx->codec_id);
	if (!codec) {
		printf("%s(%d) avcodec_find_decoder failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	//关联
	ret = avcodec_open2(codec_ctx, codec, NULL);
	if (ret < 0) {
		av_strerror(ret, err2str, sizeof(err2str));
		printf("%s(%d) avcodec_open2 failed: %d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
		return -1;
	}

	printf("%s: DecodeThread::Init() finish\n", name_);

	return 0;
}

int DecodeThread::Start()
{
	printf("%s: %s(%d)\n", name_, __FUNCTION__, __LINE__);
	// 开启线程
	thread_ = new std::thread(&DecodeThread::Run, this);
	if (!thread_) {
		printf("%s(%d) new DecodeThread failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

int DecodeThread::Stop()
{
	printf("%s: %s(%d)\n", name_, __FUNCTION__, __LINE__);
	Thread::Stop();

	if (frame_queue_->Size() != 0) {
		frame_queue_->Abort();
	}
	if (packet_queue_->Size() != 0) {
		packet_queue_->Abort();
	}

	return 0;
}

void DecodeThread::Run()
{
	printf("%s: DecodeThread::Run() into\n", name_);
	int ret = 0;
	AVFrame* frame = av_frame_alloc();

	while (true)
	{
		if (abort_ == 1) { // 线程退出标志位==1
			break;
		}

		// 队列长度控制
		if (frame_queue_->Size() > 10) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		// 从packet_queue读取packet
		AVPacket* packet = packet_queue_->Pop(10);
		if (packet) {
		// 送解码器
			ret = avcodec_send_packet(codec_ctx, packet);
			av_packet_free(&packet); // 释放（同release）
			if (ret < 0) {
				av_strerror(ret, err2str, sizeof(err2str));
				printf("%s(%d) avcodec_send_packet failed: %d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
				break;
			}
		// 从解码器读取frame
			while (true) {
				ret = avcodec_receive_frame(codec_ctx, frame);
				if (ret == AVERROR(EAGAIN)) {
					break;
				}
				else if (ret == 0) {
					// frame送入frame_queue
					frame_queue_->Push(frame);
					//printf("%s frame queue size: %d\n", codec_ctx->codec->name, frame_queue_->Size());
					continue;
				}
				else {
					abort_ = 1;
					av_strerror(ret, err2str, sizeof(err2str));
					printf("%s(%d) avcodec_receive_frame failed: %d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
					break;
				}
			}
		}
		else {// 数据包解析完毕
			printf("%s: no packet\n", name_);
			break;
		}
	}
	printf("%s: DecodeThread::Run() leave\n", name_);
}

AVCodecContext* DecodeThread::getAVCodecContext()
{
	return codec_ctx;
}
