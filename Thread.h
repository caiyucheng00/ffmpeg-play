#pragma once
#ifndef __THREAD__H_
#define __THREAD__H_

#include <thread>

/**
 * 线程基类
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
	int abort_ = 0; // 退出线程标志位
	std::thread* thread_ = nullptr; // 线程句柄
};




#endif


