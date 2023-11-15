#include "VideoOutput.h"
#include <thread>

#define REFRESH_RATE 0.01

VideoOutput::VideoOutput(AVSync* avsync, AVFrameQueue* video_frame_queue, int video_width, int video_height, AVRational time_base) :
	avsync_(avsync),
	video_frame_queue_(video_frame_queue),
	video_height_(video_height),
	video_width_(video_width),
	time_base_(time_base)
{

}


VideoOutput::~VideoOutput()
{

}

int VideoOutput::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init failed\n");
		return -1;
	}

	window_ = SDL_CreateWindow("player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		video_width_, video_height_, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!window_) {
		printf("SDL_CreateWindow failed\n");
		return -1;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, 0);
	if (!renderer_) {
		printf("SDL_CreateRenderer failed\n");
		return -1;
	}

	texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video_width_, video_height_);
	if (!renderer_) {
		printf("SDL_CreateTexture failed\n");
		return -1;
	}

	//分配
	yuv_buf_size_ = video_width_ * video_height_ * 1.5;
	yuv_buf_ = (uint8_t*)malloc(yuv_buf_size_);

	return 0;
}

int VideoOutput::MainLoop()
{
	while (true)
	{
		//读取事件
		RefreshLoopWaitEvent(&event_);
		if (finish_ == 1) {
			printf("播放片段结束\n");
			return 0;
		}
		switch (event_.type)
		{
		case SDL_KEYDOWN:
			if (event_.key.keysym.sym == SDLK_ESCAPE) {
				printf("!!!!!!!!!!!!!!!!ESC key down\n");
				return 0;
			}
			break;
		case SDL_QUIT:
			printf("SDL_QUIT\n");
			return 0;
			break;
		default:
			break;
		}
	}
}

void VideoOutput::RefreshLoopWaitEvent(SDL_Event* event)
{
	double remianing_time = 0.0;
	SDL_PumpEvents();
	while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) { //没有事件产生
		if (remianing_time > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(remianing_time * 1000)));
		}
		RefreshVideo(remianing_time);
		if (finish_ == 1) {
			break;
		}
		SDL_PumpEvents();
	}
}

void VideoOutput::RefreshVideo(double& remianing_time)
{
	AVFrame* frame = video_frame_queue_->Front();
	if (frame) {
		pts_ = frame->pts * av_q2d(time_base_);
		double diff = pts_ - avsync_->GetClock();
		if (diff > 0) {
			remianing_time = diff;
			if (remianing_time > REFRESH_RATE) {
				remianing_time = REFRESH_RATE;
			}
			return;
		}
		printf("video pts:%0.3lf diff:%0.3f\n", pts_, diff);

		rect_.x = 0;
		rect_.y = 0;
		rect_.w = video_width_;
		rect_.h = video_height_;

		SDL_UpdateYUVTexture(texture_, &rect_, frame->data[0], frame->linesize[0],
			frame->data[1], frame->linesize[1],
			frame->data[2], frame->linesize[2]);
		SDL_RenderClear(renderer_);
		SDL_RenderCopy(renderer_, texture_, NULL, &rect_);
		SDL_RenderPresent(renderer_);

		frame = video_frame_queue_->Pop(2);
		av_frame_free(&frame);
	}
	else {
		// 播完了
		finish_ = 1;
	}
}
