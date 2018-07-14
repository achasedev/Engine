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

// Console Commands
// Does time consuming work, to test thread use
void ThreadTest(void *) 
{
	// Open a file and output about 50MB of random numbers to it; 
	int* buffer = (int*) malloc(sizeof(int) * 100000000);

	for (int i = 0; i < 100000000; ++i)
	{
		buffer[i] = 270;
	}

	FileWriteFromBuffer("Data/Logs/garbage.dat", (const char*) buffer, sizeof(int) * 100000000);
	DebuggerPrintf( "Finished thread work." ); 
}

// Does time-consuming work on the main thread
void Command_RunWorkOnMainThread(Command& cmd)
{
	UNUSED(cmd);
	ConsolePrintf("Doing work on main thread (should lag)...");
	ThreadTest(nullptr);
}

// Does time consuming work on a separate thread
void Command_RunWorkOnNewThread(Command& cmd)
{
	UNUSED(cmd);

	ConsolePrintf("Doing work on new thread (should NOT lag)...");
	Thread::CreateAndDetach(ThreadTest);
}

// End console commands


//-----------------------------------------------------------------------------------------------
// Registers the console commands related to threading to the command system
//
void Thread::RegisterConsoleCommands()
{
	Command::Register("thread_test_main", "Runs a lot of work on the main thread", Command_RunWorkOnMainThread);
	Command::Register("thread_test_new", "Runs a lot of work on a new thread", Command_RunWorkOnNewThread);
}


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
