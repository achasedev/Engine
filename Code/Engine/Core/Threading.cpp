#include <chrono>
#include "Engine/Core/Threading.hpp"

ThreadHandle_t Thread::Create(Thread_cb cb, void *paramData /*= nullptr */)
{
	ThreadHandle_t handle = std::thread(cb, paramData);
	return handle;
}

void Thread::Join(ThreadHandle_t handle)
{
	handle.join();
}

void Thread::Detach(ThreadHandle_t handle)
{
	handle.detach();
}

void Thread::CreateAndDetach(Thread_cb cb, void *paramData /*= nullptr*/)
{
	ThreadHandle_t handle = Create(cb, paramData);
	handle.detach();
}

void Thread::SleepThisThreadFor(unsigned int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void Thread::YieldThisThread()
{
	std::this_thread::yield();
}
