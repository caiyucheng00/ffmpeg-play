#include "Thread.h"

Thread::Thread()
{
	printf("Thread\n");
}

Thread::~Thread()
{
	printf("~Thread\n");
}

int Thread::Start()
{
	return 0;
}

int Thread::Stop()
{
	printf("%s(%d)\n", __FUNCTION__, __LINE__);
	abort_ = 1; // ±êÖ¾Î»
	if (thread_) {
		thread_->join();
		delete thread_;
		thread_ = nullptr;
	}

	return 0;
}
