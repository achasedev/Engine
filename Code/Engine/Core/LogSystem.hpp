/************************************************************************/
/* File: LogSystem.hpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: Class to represent the static Log System
/************************************************************************/
#pragma once
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/DataStructures/ThreadSafeQueue.hpp"
#include <shared_mutex>
#include "Engine/Core/EngineCommon.hpp"

class File;

// Struct to represent a single log
struct LogMessage_t
{
	std::string tag;
	std::string message;
};

// Log callback, for hooking into processed messages
typedef void (*Log_cb)(LogMessage_t, void *paramData); 

// Struct for representing a callback with argument data
struct LogCallBack_t
{
	LogCallBack_t(Log_cb callbackP, void* argumentDataP)
	 : callback(callbackP), argumentData(argumentDataP) {}

	Log_cb	callback;
	void*	argumentData;
};


class LogSystem
{
public:
	//-----Public Methods-----

	// Initialization
	static void Initialize();
	static void Shutdown();

	// Accessors
	static bool IsRunning();

	// Mutators
	static void AddLog(LogMessage_t message);
	static void AddCallback(LogCallBack_t callback);


private:
	//-----Private Methods-----

	// static class, no instantiation
	LogSystem() = delete;
	~LogSystem() = delete;
	LogSystem(const LogSystem& copy) = delete;

	// Log thread functions
	static void ProcessLog(void*);
	static void FlushMessages();


private:
	//-----Private Data-----

	// For writing to the log file
	static File* s_logFile;
	static bool s_isRunning;
	static ThreadHandle_t s_logThread;
	static ThreadSafeQueue<LogMessage_t> s_logQueue;
	
	// Callbacks
	static std::shared_mutex s_callbackLock;
	static std::vector<LogCallBack_t> s_callbacks;

	// Statics
	static LogSystem* s_instance;
	static const char* LOG_FILE_NAME_FORMAT;

};


//////////////////////////////////////////////////////////////////////////
// C Functions
//////////////////////////////////////////////////////////////////////////

// For adding messages to the log
void LogPrintf(char const* tag, char const *format, ...);
void LogPrintv(char const* tag, char const* format, va_list args);
