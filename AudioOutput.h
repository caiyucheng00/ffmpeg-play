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
    int freq; //������
    AVChannelLayout ch_layout; // ����
    enum AVSampleFormat fmt;  // ������ʽ
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

public: // ���б���
    AVSync* avsync_ = NULL;
    AVFrameQueue* audio_frame_queue_ = NULL;
    AudioParams src_params_; // ������Դ��ʽ
    AudioParams dst_params_; // SDL�����Ҫ��ʽ
    AVRational time_base_;

    struct SwrContext* swr_ctx_ = NULL;

    uint8_t* audio_buf1_ = NULL;
    uint32_t audio_buf1_size_ = 0;   // ��������Ĵ�С�����ڵ�������
	uint8_t* audio_buf_ = NULL;
	uint32_t audio_buf_size_ = 0;   // �����ز�����ռ��
    uint32_t audio_buf_index_ = 0;

    double pts_ = AV_NOPTS_VALUE;
};

#endif
