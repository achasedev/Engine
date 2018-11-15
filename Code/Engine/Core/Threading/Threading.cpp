/************************************************************************/
/* File: Threading.cpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: Implementation of the threading API
/************************************************************************/
#include <chrono>
#include "Engine/Core/File.hpp"
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"


//-----------------------------------------------------------------------------------------------
// Creates a thread, returning the handle
//
ThreadHandle_t Thread::Create(Thread_cb cb, void *paramData /*= nullptr */)
{
	ThreadHandle_t handle = new std::thread(cb, paramData);
	return handle;
}


//-----------------------------------------------------------------------------------------------
// Makes the calling thread wait/block until the thread given by handle finishes
//
void Thread::Join(ThreadHandle_t handle)
{
	handle->join();
}


//-----------------------------------------------------------------------------------------------
// Releases the handle on the calling thread, making the thread given by handle auto-terminate
// when it finishes
//
void Thread::Detach(ThreadHandle_t handle)
{
	handle->detach();
}


//-----------------------------------------------------------------------------------------------
// Creates a new thread running the callback, and detaches it
//
void Thread::CreateAndDetach(Thread_cb cb, void *paramData /*= nullptr*/)
{
	ThreadHandle_t handle = Create(cb, paramData);
	handle->detach();
}


//-----------------------------------------------------------------------------------------------
// Makes the calling thread sleep for the given number of milliseconds
//
void Thread::SleepThisThreadFor(unsigned int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


//-----------------------------------------------------------------------------------------------
// Makes the calling thread yield their CPU time
//
void Thread::YieldThisThread()
{
	std::this_thread::yield();
}
