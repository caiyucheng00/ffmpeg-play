#pragma once
#ifndef __AUDIOOUTPUT__H_
#define __AUDIOOUTPUT__H_

#include "AVFrameQueue.h"
#include "AVSync.h"

#ifdef __cplusplus
extern "C" {
    #include <SDL.h>
    #include <libswresample/swresample.h>
}
#endif

typedef struct AudioParams{
    int freq; //采样率
    AVChannelLayout ch_layout; // 布局
    enum AVSampleFormat fmt;  // 采样格式
    int channels;
    int frame_size;
} AudioParams;

class AudioOutput
{
public:
    AudioOutput(AVSync* avsync, const AudioParams& audio_params, AVFrameQueue* audio_frame_queue, AVRational time_base);
	~AudioOutput();

    int Init();
    int DeInit();

public: // 公有变量
    AVSync* avsync_ = NULL;
    AVFrameQueue* audio_frame_queue_ = NULL;
    AudioParams src_params_; // 解码后的源格式
    AudioParams dst_params_; // SDL输出需要格式
    AVRational time_base_;

    struct SwrContext* swr_ctx_ = NULL;

    uint8_t* audio_buf1_ = NULL;
    uint32_t audio_buf1_size_ = 0;   // 真正分配的大小，大于等于下面
	uint8_t* audio_buf_ = NULL;
	uint32_t audio_buf_size_ = 0;   // 真正重采样后占用
    uint32_t audio_buf_index_ = 0;

    double pts_ = AV_NOPTS_VALUE;
};

#endif
