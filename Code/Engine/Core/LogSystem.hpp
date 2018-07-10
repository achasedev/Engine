#pragma once
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/DataStructures/ThreadSafeQueue.hpp"
#include <shared_mutex>
#include "Engine/Core/EngineCommon.hpp"
TODO("Comment");
class File;

struct LogMessage_t
{
	std::string tag;
	std::string message;
};

typedef void (*Log_cb)(LogMessage_t, void *paramData); 

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

	static void Initialize();
	static void Shutdown();

	static bool IsRunning();

	static void AddLog(LogMessage_t message);
	static void AddCallback(LogCallBack_t callback);


private:
	//-----Private Methods-----

	LogSystem() = delete;
	~LogSystem() = delete;
	LogSystem(const LogSystem& copy) = delete;

	static void ProcessLog(void*);
	static void FlushMessages();



private:
	//-----Private Data-----

	// For writing to the log file
	static File* s_logFile;
	static ThreadHandle_t s_logThread;
	
	static std::shared_mutex s_callbackLock;
	static std::vector<LogCallBack_t> s_callbacks;

	static ThreadSafeQueue<LogMessage_t> s_logQueue;
	static bool s_isRunning;

	// Statics
	static LogSystem* s_instance;
	static const char* LOG_FILE_NAME_FORMAT;

};


//////////////////////////////////////////////////////////////////////////
// C Functions
//////////////////////////////////////////////////////////////////////////

void LogPrintf(char const* tag, char const *format, ...);
void LogPrintv(char const* tag, char const* format, va_list args);
