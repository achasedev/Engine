#pragma once
#include <thread>

// I'm looking for a file *like* this, not does not need to be
// exact.  Should have the basic thread functions in an easy to 
// find structure. 
//
// Even if you prefer class based Threads, provide the C functions,
// as the class based ones will use them anyway;
//
// HOW you implement them under the hood I don't care
// I use Win32 Functions, but std::thread is fine.  
// DO NOT USE BOOST!

typedef std::thread* ThreadHandle_t; 

// Thread callback function (what the thread enters into)
typedef void (*Thread_cb)(void *paramData); 

namespace Thread
{
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
