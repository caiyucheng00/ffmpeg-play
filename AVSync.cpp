#include "AVSync.h"


AVSync::AVSync()
{

}

AVSync::~AVSync()
{

}

void AVSync::InitClock()
{
	SetClock(NAN);
}

void AVSync::SetClock(double pts)
{
	double time = GetMicroseconds() / 1000000.0;
	pts_drift_ = pts - time;
}

double AVSync::GetClock()
{
	double time = GetMicroseconds() / 1000000.0;
	return pts_drift_ + time;
}

time_t AVSync::GetMicroseconds()
{
	system_clock::time_point time_point_now = system_clock::now();
	system_clock::duration duration = time_point_now.time_since_epoch();
	time_t us = duration_cast<microseconds>(duration).count();

	return us;
}
