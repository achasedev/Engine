/************************************************************************/
/* File: LogSystem.hpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: Class to represent the static Log System
/************************************************************************/
#pragma once
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/DataStructures/ThreadSafeSet.hpp"
#include "Engine/DataStructures/ThreadSafeMap.hpp"
#include "Engine/DataStructures/ThreadSafeQueue.hpp"
#include <shared_mutex>

class File;

// Struct to represent a single log
struct LogMessage_t
{
	LogMessage_t() {}
	LogMessage_t(const char* _tag, const char* _message)
	 : tag(_tag), message(_message) {}

	std::string tag;
	std::string message;

};

// Log callback, for hooking into processed messages
typedef void (*Log_cb)(LogMessage_t, void *paramData); 

// Struct for representing a callback with argument data
struct LogCallBack_t
{
	LogCallBack_t() {}
	LogCallBack_t(const char* _name, Log_cb _callback, void* _argumentData)
	 : name(_name), callback(_callback), argumentData(_argumentData) {}

	std::string		name;
	Log_cb			callback;
	void*			argumentData;

};

// Struct for a callback with a filtered list
// This allows us to pass the LogCallback_t around by value, since
// inside the set the std::mutex can't be copied (thus, can't copy LogFilteredCallback_t)
struct LogFilteredCallback_t
{
	LogFilteredCallback_t() {}
	LogFilteredCallback_t(LogCallBack_t logCallback)
	 : logCallback(logCallback) {}

	LogCallBack_t				logCallback;
	ThreadSafeSet<std::string>	filters;
	bool						isBlackList = true;
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
	static void AddCallback(const char* name, Log_cb callback, void* argumentData);
	static void FlushLog();

	static void AddCallbackFilter(const std::string& callbackName, const std::string& filter);
	static void RemoveCallBackFilter(const std::string& callbackName, const std::string& filter);
	static void SetCallbackToBlackList(const std::string& callbackName, bool isBlackList);

	static void ShowAllTags();
	static void HideAllTags();


private:
	//-----Private Methods-----

	// static class, no instantiation
	LogSystem() = delete;
	~LogSystem() = delete;
	LogSystem(const LogSystem& copy) = delete;

	static void InitializeConsoleCommands();


	// Log thread functions
	static void ProcessLog(void*);
	static void ProcessAllLogsInQueue();


private:
	//-----Private Data-----

	// For writing to the log file
	static File* s_latestLogFile;
	static File* s_timeStampedFile;

	static bool s_isRunning;
	static ThreadHandle_t s_logThread;
	static ThreadSafeQueue<LogMessage_t> s_logQueue;
	
	// Callbacks
	static std::shared_mutex s_callbackLock;
	static std::map<std::string, LogFilteredCallback_t> s_callbacks;

	// Statics
	static LogSystem* s_instance;
	static const char* LOG_FILE_NAME_FORMAT;

};


//////////////////////////////////////////////////////////////////////////
// C Functions
//////////////////////////////////////////////////////////////////////////

// For adding messages to the log
void LogPrintf(char const *format, ...);
void LogPrintv(char const* format, va_list args);

void LogTaggedPrintf(char const* tag, char const *format, ...);
void LogTaggedPrintv(char const* tag, char const* format, va_list args);
