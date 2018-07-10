/************************************************************************/
/* File: LogSystem.cpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: Implementation of the LogSystem class
/************************************************************************/
#include <stdarg.h>
#include "Engine/Core/File.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"

// For LogPrintf, ensuring we don't copy anything too large
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

// Static members
bool							LogSystem::s_isRunning = true;
File*							LogSystem::s_logFile = nullptr;
const char*						LogSystem::LOG_FILE_NAME_FORMAT = "Data/Logs/SystemLog_%s.txt";
ThreadHandle_t					LogSystem::s_logThread = nullptr;
std::shared_mutex				LogSystem::s_callbackLock;
std::vector<LogCallBack_t>		LogSystem::s_callbacks;
ThreadSafeQueue<LogMessage_t>	LogSystem::s_logQueue;

// Callback for writing the log to the system file
static void WriteToFile(LogMessage_t log, void* fileptr);


//-----------------------------------------------------------------------------------------------
// Initializes the system
//
void LogSystem::Initialize()
{
	s_logFile = new File();
	std::string logFileName = Stringf(LOG_FILE_NAME_FORMAT, GetSystemDateAndTime().c_str());
	s_logFile->Open(logFileName.c_str(), "a+");

	AddCallback(LogCallBack_t(WriteToFile, s_logFile));

	s_logThread = Thread::Create(&ProcessLog, nullptr);

	s_isRunning = true;
}


//-----------------------------------------------------------------------------------------------
// Shuts down and cleans up the system
//
void LogSystem::Shutdown()
{
	s_isRunning = false;

	// Flush the rest of the log
	Thread::Join(s_logThread);
	s_logThread = nullptr;

	s_logFile->Close();
}


//-----------------------------------------------------------------------------------------------
// Returns true if the system is active, with the log thread currently running
//
bool LogSystem::IsRunning()
{
	return s_isRunning;
}


//-----------------------------------------------------------------------------------------------
// Adds the given log message to the LogSystem
//
void LogSystem::AddLog(LogMessage_t message)
{
	s_logQueue.Enqueue(message);
}


//-----------------------------------------------------------------------------------------------
// Adds the callback hook to the list of callbacks to call when a message is processed
//
void LogSystem::AddCallback(LogCallBack_t callback)
{
	s_callbackLock.lock();
	s_callbacks.push_back(callback);
	s_callbackLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Log Thread
// Constantly spins/sleeps, processing and removing messages in the queue
//
void LogSystem::ProcessLog(void*)
{
	while (IsRunning())
	{
		FlushMessages();
		Thread::SleepThisThreadFor(10); TODO("Use a semaphore here")
	}

	// Ensure the last of the messages are processed before terminating
	FlushMessages();
}


//-----------------------------------------------------------------------------------------------
// Processes the messages in the queue, emptying it
//
void LogSystem::FlushMessages()
{
	LogMessage_t message;
	while (s_logQueue.Dequeue(message)) // Returns false when the queue is empty
	{
		s_callbackLock.lock_shared();

		for (int callbackIndex = 0; callbackIndex < (int) s_callbacks.size(); ++callbackIndex)
		{
			s_callbacks[callbackIndex].callback(message, s_callbacks[callbackIndex].argumentData);
		}

		s_callbackLock.unlock_shared();
	}
}


//-----------------------------------------------------------------------------------------------
// Callback for writing a log to the log file
//
static void WriteToFile(LogMessage_t log, void* fileptr)
{
	File* file = (File*) fileptr;
	std::string toPrint = Stringf("%s: %s\n", log.tag.c_str(), log.message.c_str());
	file->Write(toPrint.c_str(), toPrint.size());
}


//-----------------------------------------------------------------------------------------------
// Adds the given tag and text to the log system as a log
//
void LogPrintv(char const* tag, char const* format, va_list args)
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s(textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, args);	
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)
													
	LogMessage_t log;
	log.message	= std::string(textLiteral);
	log.tag = tag;

	LogSystem::AddLog(log);
}


//-----------------------------------------------------------------------------------------------
// Adds the given tag and text to the log system as a log
//
void LogPrintf(char const* tag, char const *format, ...)
{
	// Construct the string
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogPrintv(tag, format, variableArgumentList);
	va_end(variableArgumentList);
}
