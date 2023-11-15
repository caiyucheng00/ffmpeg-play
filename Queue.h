#pragma once
#ifndef __QUEUE__H_
#define __QUEUE__H_

#include <mutex>
#include <condition_variable>
#include <queue>

using namespace std;

template <typename T>
class Queue
{
public:
	Queue();
	~Queue();

	void Abort();
	int Push(T val);
	int Pop(T &val, const int timeout = 0);
	int Front(T& val);
	int Size();

private: 
	int abort_ = 0;
	mutex mutex_;
	condition_variable cond_;
	queue<T> queue_;
};

template <typename T>
Queue<T>::Queue()
{

}

template <typename T>
Queue<T>::~Queue()
{

}

template <typename T>
void Queue<T>::Abort()
{
	// ��־λ=1
	abort_ = 1;
	// ����
	cond_.notify_all();
}

template <typename T>
int Queue<T>::Push(T val)
{
	// ��
	lock_guard<mutex> lock(mutex_);
	if (1 == abort_) return -1;

	queue_.push(val);
	cond_.notify_one();

	return 0;
}

template <typename T>
int Queue<T>::Pop(T& val, const int timeout)
{
	// ��
	unique_lock<mutex> lock(mutex_);
	if (queue_.empty()) { // ���п��ˣ�����:��ʱ����or������������
		cond_.wait_for(lock, chrono::microseconds(timeout), [this] {return !queue_.empty() | (abort_ == 1); });
	}
	if (1 == abort_) return -1;  // �ж�
	if (queue_.empty()) return -2; // �ն���

	val = queue_.front();
	queue_.pop();

	return 0;
}

template <typename T>
int Queue<T>::Front(T& val)
{
	// ��
	lock_guard<mutex> lock(mutex_);
	if (1 == abort_) return -1;  // �ж�
	if (queue_.empty()) return -2; // �ն���

	val = queue_.front();

	return 0;
}

template <typename T>
int Queue<T>::Size()
{
	// ��
	lock_guard<mutex> lock(mutex_);
	return queue_.size();
}

#endif