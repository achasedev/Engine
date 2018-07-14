/************************************************************************/
/* File: Threading.hpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: API for creating and manipulating threads
/************************************************************************/
#pragma once
#include <thread>
#include "Engine/Core/EngineCommon.hpp"

TODO("Return handles as void*")
TODO("Add name thread functionality");
typedef std::thread* ThreadHandle_t; 

// Thread callback function (what the thread enters into)
typedef void (*Thread_cb)(void *paramData); 

namespace Thread
{
	// For console commands
	void RegisterConsoleCommands();

	// Creating
	ThreadHandle_t Create( Thread_cb cb, void *user_data = nullptr ); 

	// Releasing - how we free an above resource
	// Join will wait until the thread is complete before return control to the calling thread
	void Join(ThreadHandle_t handle); 

	// Detach will return control immediately, and the thread will be freed automatically when complete;
	void Detach(ThreadHandle_t handle); 

	// For starting a thread without needing to track it
	void CreateAndDetach(Thread_cb cb, void *user_data = nullptr); 

	// Control
	void SleepThisThreadFor(unsigned int ms); 
	void YieldThisThread(); 
}
