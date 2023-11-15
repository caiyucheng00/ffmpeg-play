#pragma once
#ifndef __VIDEOOUTPUT__H_
#define __VIDEOOUTPUT__H_

#include "AVFrameQueue.h"
#include "AVSync.h"

#ifdef __cplusplus
extern "C" {
#include <SDL.h>
#include <libswresample/swresample.h>
}
#endif

class VideoOutput
{
public:
	VideoOutput(AVSync* avsync, AVFrameQueue* video_frame_queue, int video_width, int video_height, AVRational time_base);
	~VideoOutput();

	int Init();
	int MainLoop();
	void RefreshLoopWaitEvent(SDL_Event* event);

private:
	void RefreshVideo(double& remianing_time);

	AVSync* avsync_ = NULL;
	AVFrameQueue* video_frame_queue_ = NULL;
	int video_width_;
	int video_height_;
	AVRational time_base_;

	SDL_Event event_;
	SDL_Window* window_ = NULL;
	SDL_Renderer* renderer_ = NULL;
	SDL_Texture* texture_ = NULL;
	SDL_Rect rect_;

	uint8_t* yuv_buf_ = NULL;
	int yuv_buf_size_ = 0;

	double pts_ = AV_NOPTS_VALUE;
	int finish_ = 0;
};

#endif