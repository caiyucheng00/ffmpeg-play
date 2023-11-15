#pragma once
#ifndef __THREAD__H_
#define __THREAD__H_

#include <thread>

/**
 * �̻߳���
 */
class Thread
{
public:
	Thread();
	virtual ~Thread();

	virtual int Start();
	virtual int Stop();
	virtual void Run() = 0;

protected:
	int abort_ = 0; // �˳��̱߳�־λ
	std::thread* thread_ = nullptr; // �߳̾��
};




#endif


