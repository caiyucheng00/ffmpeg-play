#pragma once
#ifndef __ASYNC__H_
#define __ASYNC__H_

#include <chrono>
#include <ctime>

using namespace std::chrono;

class AVSync
{
public:
	AVSync();
	~AVSync();

	void InitClock();
	void SetClock(double pts);
	double GetClock();

private:
	time_t GetMicroseconds();

	double pts_drift_ = 0;
};

#endif
