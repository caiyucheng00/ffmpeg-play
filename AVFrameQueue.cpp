#include "AVFrameQueue.h"

AVFrameQueue::AVFrameQueue()
{

}

AVFrameQueue::~AVFrameQueue()
{

}

void AVFrameQueue::Abort()
{
	release();
	queue_.Abort();
}

int AVFrameQueue::Push(AVFrame* val)
{
	AVFrame* tmp_frame = av_frame_alloc();
	av_frame_move_ref(tmp_frame, val);
	return queue_.Push(tmp_frame);
}

AVFrame* AVFrameQueue::Pop(const int timeout)
{
	AVFrame* tmp_frame = NULL;
	int ret = queue_.Pop(tmp_frame, timeout);
	if (ret < 0) {
		if (ret == -1) {
			printf("FrameQueue abort\n");
		}
	}

	return tmp_frame;
}

AVFrame* AVFrameQueue::Front()
{
	AVFrame* tmp_frame = NULL;
	int ret = queue_.Front(tmp_frame);
	if (ret < 0) {
		if (ret == -1) {
			printf("FrameQueue abort\n");
		}
	}

	return tmp_frame;
}

int AVFrameQueue::Size()
{
	return queue_.Size();
}

/**
 * 队列中AVFrame释放
 */
void AVFrameQueue::release()
{
	while (true)
	{
		AVFrame* tmp_frame = NULL;
		int ret = queue_.Pop(tmp_frame, 1);
		if (ret < 0) {
			break;
		}
		else // 队列中还有AVFrame
		{
			av_frame_free(&tmp_frame);
			continue;
		}
	}
}
