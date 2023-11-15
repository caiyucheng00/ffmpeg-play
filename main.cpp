#include <iostream>
#include "DemuxThread.h"
#include "AVPacketQueue.h"
#include "DecodeThread.h"
#include "AudioOutput.h"
#include "VideoOutput.h"
#include "AVSync.h"

using namespace std;

int main(int argc, char* argv[]) {
	cout << "主函数开始运行" << endl;

	printf("url: %s\n", argv[1]);

	int ret = 0;
	AVPacketQueue* audio_packet_queue = new AVPacketQueue;
	AVPacketQueue* video_packet_queue = new AVPacketQueue;
	AVFrameQueue* audio_frame_queue = new AVFrameQueue;
	AVFrameQueue* video_frame_queue = new AVFrameQueue;

	AVSync* avsync = new AVSync;

	DemuxThread* demux_thread = new DemuxThread(audio_packet_queue, video_packet_queue);
	ret = demux_thread->Init(argv[1]);
	if (ret < 0) {
		printf("%s(%d) DemuxThread Init failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = demux_thread->Start();
	if (ret < 0) {
		printf("%s(%d) DemuxThread Start failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	//this_thread::sleep_for(chrono::microseconds(30000));

	// 音频解码
	DecodeThread* audio_decode_thread = new DecodeThread(audio_packet_queue, audio_frame_queue, "Audio");
	ret = audio_decode_thread->Init(demux_thread->AudioCodecParameters());
	if (ret < 0) {
		printf("%s(%d) audio_decode_thread Init failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = audio_decode_thread->Start();
	if (ret < 0) {
		printf("%s(%d) audio_decode_thread Start failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}
	// 视频解码
	DecodeThread* video_decode_thread = new DecodeThread(video_packet_queue, video_frame_queue, "Video");
	ret = video_decode_thread->Init(demux_thread->VideoCodecParameters());
	if (ret < 0) {
		printf("%s(%d) video_decode_thread Init failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = video_decode_thread->Start();
	if (ret < 0) {
		printf("%s(%d) video_decode_thread Start failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	avsync->InitClock();

	// 音频输出
	AudioParams audio_params;  //src 源
	memset(&audio_params, 0, sizeof(audio_params));
	//填充src_params
	audio_params.ch_layout = audio_decode_thread->getAVCodecContext()->ch_layout;
	audio_params.fmt = audio_decode_thread->getAVCodecContext()->sample_fmt;
	audio_params.freq = audio_decode_thread->getAVCodecContext()->sample_rate;
	audio_params.channels = audio_decode_thread->getAVCodecContext()->ch_layout.nb_channels;
	audio_params.frame_size = audio_decode_thread->getAVCodecContext()->frame_size;
	AudioOutput* audio_output = new AudioOutput(avsync, audio_params, audio_frame_queue, demux_thread->AudioStreamTimebase());
	ret = audio_output->Init();
	if (ret < 0) {
		printf("%s(%d) audio_output Init failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}

	// 视频输出
	VideoOutput* video_output = new VideoOutput(avsync, video_frame_queue, demux_thread->VideoCodecParameters()->width, demux_thread->VideoCodecParameters()->height, demux_thread->VideoStreamTimebase());
	video_output->Init();
	if (ret < 0) {
		printf("%s(%d) video_output Init failed: \n", __FUNCTION__, __LINE__);
		return -1;
	}
	video_output->MainLoop();

	audio_output->DeInit();
	delete audio_output;

	printf("audio_frame_queue: %d\n", audio_frame_queue->Size());
	printf("video_frame_queue: %d\n", video_frame_queue->Size());
	printf("audio_packet_queue: %d\n", audio_packet_queue->Size());
	printf("video_packet_queue: %d\n", video_packet_queue->Size());

	delete avsync;

	demux_thread->Stop();
	delete demux_thread;

	audio_decode_thread->Stop();
	delete audio_decode_thread;
	video_decode_thread->Stop();
	delete video_decode_thread;
	printf("audio_frame_queue: %d\n", audio_frame_queue->Size());
	printf("video_frame_queue: %d\n", video_frame_queue->Size());
	printf("audio_packet_queue: %d\n", audio_packet_queue->Size());
	printf("video_packet_queue: %d\n", video_packet_queue->Size());
	delete audio_frame_queue;
	delete video_frame_queue;
	delete audio_packet_queue;
	delete video_packet_queue;

	cout << "主函数结束运行" << endl;
	return 0;
}