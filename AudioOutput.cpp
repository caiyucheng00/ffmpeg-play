#include "AudioOutput.h"

AudioOutput::AudioOutput(AVSync* avsync, const AudioParams& audio_params, AVFrameQueue* audio_frame_queue, AVRational time_base) :
	avsync_(avsync),
	src_params_(audio_params),
	audio_frame_queue_(audio_frame_queue),
	time_base_(time_base)
{

}

AudioOutput::~AudioOutput()
{

}

// 回调函数 
void audio_callback(void* userdata, Uint8 * stream, int len) {
	AudioOutput* audio_output = (AudioOutput*)userdata;

	//printf("==================================================================len: %d\n", len); //4096

	while (len > 0)
	{
		if(audio_output->audio_buf_index_ == audio_output->audio_buf_size_) { //上次audio_buf_数据已经读完
		// 1.读取pcm数据
			audio_output->audio_buf_index_ = 0;  //重置index
			AVFrame * frame = audio_output->audio_frame_queue_->Pop(2);  //frame=源
			if (frame) {
				audio_output->pts_ = frame->pts * av_q2d(audio_output->time_base_);
				printf("audio pts:%0.3lf\n", audio_output->pts_);

				// 2.重采样 和输出的不一样
				if ((frame->format != audio_output->dst_params_.fmt) ||
					(frame->sample_rate != audio_output->dst_params_.freq) ||
					(av_channel_layout_compare(&frame->ch_layout, &audio_output->dst_params_.ch_layout)) != 0 &&
					(!audio_output->swr_ctx_)) {
					// 仅仅初始化一次
					swr_alloc_set_opts2(&audio_output->swr_ctx_,
						&audio_output->dst_params_.ch_layout, audio_output->dst_params_.fmt, audio_output->dst_params_.freq,
						&frame->ch_layout, (enum AVSampleFormat)frame->format, frame->sample_rate,
						0, NULL);
					if (!audio_output->swr_ctx_ || swr_init(audio_output->swr_ctx_) < 0) {
						printf("swr_init failed\n");
						if (audio_output->swr_ctx_) {
							swr_free(&audio_output->swr_ctx_);
						}
						return;
					}
				}


				if (audio_output->swr_ctx_) {
					// 需要重采样
					const uint8_t** in = (const uint8_t**)frame->extended_data;
					uint8_t** out = &audio_output->audio_buf1_;
					//最大输出样本  8khz 1024个样本 --->> 16khz ？个样本(2048)                                + 缓存
					int out_samples = frame->nb_samples * audio_output->dst_params_.freq / frame->sample_rate + 256;
					int out_bytes = av_samples_get_buffer_size(NULL,
						audio_output->dst_params_.ch_layout.nb_channels,
						out_samples,
						audio_output->dst_params_.fmt, 0);
					if (out_bytes < 0) {
						printf("av_samples_get_buffer_size failed\n");
						return;
					}
					// audio_buf1_size_ < out_bytes,内部重新分配数据
					av_fast_malloc(&audio_output->audio_buf1_, &audio_output->audio_buf1_size_, out_bytes);

					int convert_samples = swr_convert(audio_output->swr_ctx_, out, out_samples, in, frame->nb_samples);
					if (convert_samples < 0) {
						printf("swr_convert failed\n");
					}

					audio_output->audio_buf_size_ = av_samples_get_buffer_size(NULL,
						audio_output->dst_params_.ch_layout.nb_channels,
						convert_samples,
						audio_output->dst_params_.fmt, 0);
					audio_output->audio_buf_ = audio_output->audio_buf1_;
				} else {
					// 不需要
					// audio_buf1_size_ < out_bytes,内部重新分配数据
					int out_bytes = av_samples_get_buffer_size(NULL,
						frame->ch_layout.nb_channels,
						frame->nb_samples,
						(enum AVSampleFormat)frame->format, 0);
					av_fast_malloc(&audio_output->audio_buf1_, &audio_output->audio_buf1_size_, out_bytes);
					audio_output->audio_buf_size_ = out_bytes;
					audio_output->audio_buf_ = audio_output->audio_buf1_;
					memcpy(audio_output->audio_buf_, frame->extended_data, out_bytes);
				}

				// 一定释放
				av_frame_free(&frame);
			} else {
				// 空frame
				audio_output->audio_buf_ = NULL;
				audio_output->audio_buf_size_ = 512;
			}
		}
		// 3.audio_buf_ ----》 stream
		int left_len = audio_output->audio_buf_size_ - audio_output->audio_buf_index_;   //len是audio_buf_剩余的
		if (left_len > len) {
			left_len = len;           // 最多是 stream剩下的
		}
		if (!audio_output->audio_buf_) {
			memset(stream, 0, left_len);   // 空frame 放512个0 （line 90）
		}
		else {
			memcpy(stream, audio_output->audio_buf_ + audio_output->audio_buf_index_, left_len);
		}

		//4.len更新
		len -= left_len;  //stream剩下的len
		audio_output->audio_buf_index_ += left_len;
		stream += left_len;

		//printf("len:%d, audio_buf_index:%d, audio_buf_size:%d, left_len:%d\n", len, audio_output->audio_buf_index_, audio_output->audio_buf_size_, left_len);
	}

	// 设置时钟
	if (audio_output->pts_ != AV_NOPTS_VALUE) {
		audio_output->avsync_->SetClock(audio_output->pts_);
	}
}

int AudioOutput::Init()
{
	int ret = 0;

	// SDL初始化
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		printf("SDL_Init failed\n"); 
		return -1;
	}

	// 输出的格式
	SDL_AudioSpec wanted_spec;
	wanted_spec.channels = 2;
	wanted_spec.freq = src_params_.freq; // 预设
	wanted_spec.format = AUDIO_S16SYS;  // 必须这个格式  单通道2字节
	wanted_spec.silence = 0;           // 静音数据
	wanted_spec.callback = audio_callback;          // 回调函数
	wanted_spec.userdata = this;
	wanted_spec.samples = 1024;//src_params_.frame_size; // 1024个样本 回调一次 2*2*1024 = 4096     1024/freq=21.3ms

	ret = SDL_OpenAudio(&wanted_spec, NULL);
	if (ret != 0) {
		printf("SDL_OpenAudio failed\n");
		return -1;
	}

	//填充dst_params
	av_channel_layout_default(&dst_params_.ch_layout, wanted_spec.channels);
	dst_params_.fmt = AV_SAMPLE_FMT_S16;
	dst_params_.freq = wanted_spec.freq;

	SDL_PauseAudio(0); // 启动回调

	printf("AudioOutput::Init() finish\n");
	return 0;
}

int AudioOutput::DeInit()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	printf("AudioOutput::DeInit() finish\n");
	return 0;
}
