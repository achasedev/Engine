/************************************************************************/
/* File: LogSystem.cpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: Implementation of the LogSystem class
/************************************************************************/
#include <stdarg.h>
#include <iostream>
#include "Engine/Core/File.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"

// For writing to the output pane
#if defined( _WIN32 )
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

TODO("Make this singleton");

// For LogPrintf, ensuring we don't copy anything too large
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

// Static members
bool											LogSystem::s_isRunning = true;
File*											LogSystem::s_latestLogFile = nullptr;
File*											LogSystem::s_timeStampedFile = nullptr;
const char*										LogSystem::LOG_FILE_NAME_FORMAT = "Data/Logs/SystemLog_%s.txt";
ThreadHandle_t									LogSystem::s_logThread = nullptr;
std::shared_mutex								LogSystem::s_callbackLock;
ThreadSafeQueue<LogMessage_t>					LogSystem::s_logQueue;
std::map<std::string, LogFilteredCallback_t>	LogSystem::s_callbacks;

// Callback for writing the log to the system file
static void WriteToFile(LogMessage_t log, void* fileptr);
static void WriteToDebugOutput(LogMessage_t log, void* fileptr);
static void Command_TestFlood(Command& cmd);
static void Command_TestFlush(Command& cmd);
static void Command_ShowAllTags(Command& cmd);
static void Command_HideAllTags(Command& cmd);


//-----------------------------------------------------------------------------------------------
// Initializes the system
//
void LogSystem::Initialize()
{
	// Ensure the directory we need for the files exists
	CreateDirectoryA("Data/Logs", NULL);

	// Make the file objects
	s_latestLogFile = new File();

	// Get the paths, and open the files
	std::string latestName = "Data/Logs/SystemLog.txt";
	bool success = s_latestLogFile->Open(latestName.c_str(), "w+");

	// If we can't open the latest log file, that means this is a client, so open a different log file
	if (!success)
	{
		latestName = "Data/Logs/SystemLog_Client.txt";
		s_latestLogFile->Open(latestName.c_str(), "w+");
	}

	// Make a file writer for the latest log file
	LogCallBack_t writerCallback;
	writerCallback.callback = WriteToFile;
	writerCallback.name = "File_writer_current";
	writerCallback.argumentData = s_latestLogFile;
	AddCallback(writerCallback);

	// Make a file writer callback for the time stamped log file
	s_timeStampedFile = new File();
	std::string timeStampedName = Stringf(LOG_FILE_NAME_FORMAT, GetFormattedSystemDateAndTime().c_str());
	s_timeStampedFile->Open(timeStampedName.c_str(), "a+");

	writerCallback.name = "File_writer_timestamped";
	writerCallback.argumentData = s_timeStampedFile;
	writerCallback.callback = WriteToFile;
	AddCallback(writerCallback);

	// For printing logs to output - only listen for DEBUG tags to avoid spamming
	LogCallBack_t debugCallback;
	debugCallback.name = "Debug Output";
	debugCallback.argumentData = nullptr;
	debugCallback.callback = WriteToDebugOutput;
	AddCallback(debugCallback);
	SetCallbackToBlackList("Debug Output", false);
	AddCallbackFilter("Debug Output", "DEBUG");

	s_logThread = Thread::Create(&ProcessLog, nullptr);
	s_isRunning = true;

	// Initialize the console commands
	InitializeConsoleCommands();
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

	if (s_latestLogFile != nullptr)
	{
		s_latestLogFile->Close();
		delete s_latestLogFile;
		s_latestLogFile = nullptr;
	}

	if (s_timeStampedFile != nullptr)
	{
		s_timeStampedFile->Close();

		delete s_timeStampedFile;
		s_timeStampedFile = nullptr;
	}
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
	s_callbacks[callback.name].logCallback = callback;
	s_callbackLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Adds a callback hook to the list of callbacks to call when a message is processed
//
void LogSystem::AddCallback(const char* name, Log_cb callback, void* argumentData)
{
	AddCallback(LogCallBack_t(name, callback, argumentData));
}


//-----------------------------------------------------------------------------------------------
// Processes any remaining messages and flushes the writes to disk
// Used in case a break point or error is hit
//
void LogSystem::FlushLog()
{
	while (!s_logQueue.IsEmpty())
	{
		// Spin wait
	}
	
	// Flush the files
	s_latestLogFile->Flush();
	s_timeStampedFile->Flush();
}


//-----------------------------------------------------------------------------------------------
// Adds the given filter to the callback given by callbackName
//
void LogSystem::AddCallbackFilter(const std::string& callbackName, const std::string& filter)
{
	s_callbackLock.lock();

	std::map<std::string, LogFilteredCallback_t>::iterator itr = s_callbacks.find(callbackName);

	if (itr != s_callbacks.end())
	{
		itr->second.filters.InsertUnique(filter);
	}
	else
	{
		ERROR_AND_DIE(Stringf("Error: LogSystem::AddCallbackFilter received callback name that doesn't exist, name was \"%s\"", callbackName.c_str()));
	}

	s_callbackLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Removes the given filter to the callback given by callbackName
//
void LogSystem::RemoveCallBackFilter(const std::string& callbackName, const std::string& filter)
{
	s_callbackLock.lock();

	std::map<std::string, LogFilteredCallback_t>::iterator itr = s_callbacks.find(callbackName);

	if (itr != s_callbacks.end())
	{
		itr->second.filters.Remove(filter);
	}
	else
	{
		ERROR_AND_DIE(Stringf("Error: LogSystem::RemoveCallbackFilter received callback name that doesn't exist, name was \"%s\"", callbackName.c_str()));
	}

	s_callbackLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Sets the callback list type to the one specified by isBlackList, and also clears the filter list
//
void LogSystem::SetCallbackToBlackList(const std::string& callbackName, bool isBlackList)
{
	s_callbackLock.lock();

	std::map<std::string, LogFilteredCallback_t>::iterator itr = s_callbacks.find(callbackName);

	if (itr != s_callbacks.end())
	{
		itr->second.isBlackList = isBlackList;
		itr->second.filters.Clear();
	}
	else
	{
		ERROR_AND_DIE(Stringf("Error: LogSystem::SetCallbackToBlackList received callback name that doesn't exist, name was \"%s\"", callbackName.c_str()));
	}

	s_callbackLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Sets all callbacks to blacklist mode and clears their lists, so all tags will show
//
void LogSystem::ShowAllTags()
{
	s_callbackLock.lock();
	std::map<std::string, LogFilteredCallback_t>::iterator itr = s_callbacks.begin();

	while (itr != s_callbacks.end())
	{
		itr->second.isBlackList = true;
		itr->second.filters.Clear();
	}

	s_callbackLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Sets all callbacks to whitelist mode and clears their lists, so no tags will show
//
void LogSystem::HideAllTags()
{
	s_callbackLock.lock();
	std::map<std::string, LogFilteredCallback_t>::iterator itr = s_callbacks.begin();

	while (itr != s_callbacks.end())
	{
		itr->second.isBlackList = false;
		itr->second.filters.Clear();
	}

	s_callbackLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Registers all LogSystem console commands
//
void LogSystem::InitializeConsoleCommands()
{
	Command::Register("log_test_flood",		"Tests the log system by loading a test file and flooding the LogSystem with logs",			Command_TestFlood);
	Command::Register("log_test_flush",		"Tests the log system by sending a log message, then immediately flushing and breaking",	Command_TestFlush);
	Command::Register("log_show_all_tags",	"Enables all tags on all current callback hooks in the LogSystem",							Command_ShowAllTags);
	Command::Register("log_hide_all_tags",	"Disables all tags on all current callback hooks in the LogSystem",							Command_HideAllTags);
}


//-----------------------------------------------------------------------------------------------
// Log Thread
// Constantly spins/sleeps, processing and removing messages in the queue
//
void LogSystem::ProcessLog(void*)
{
	while (IsRunning())
	{
		ProcessAllLogsInQueue();
		Thread::SleepThisThreadFor(10); TODO("Use a semaphore here")
	}

	// Ensure the last of the messages are processed before terminating
	ProcessAllLogsInQueue();
}


//-----------------------------------------------------------------------------------------------
// Processes the messages in the queue, emptying it
//
void LogSystem::ProcessAllLogsInQueue()
{
	LogMessage_t message;
	while (s_logQueue.Dequeue(message)) // Returns false when the queue is empty
	{
		s_callbackLock.lock_shared();

		std::map<std::string, LogFilteredCallback_t>::iterator itr = s_callbacks.begin();

		for (itr; itr != s_callbacks.end(); itr++)
		{
			LogCallBack_t logCallback = itr->second.logCallback;

			// Only process the message if it's on our whitelist OR not on our blacklist, depending on our state
			if (itr->second.isBlackList && !itr->second.filters.Contains(message.tag))
			{
				logCallback.callback(message, logCallback.argumentData);
			}
			else if (!itr->second.isBlackList && itr->second.filters.Contains(message.tag))
			{
				logCallback.callback(message, logCallback.argumentData);
			}
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
	std::string toPrint = Stringf("[%s] %s: %s\n", GetFormattedSystemTime().c_str(), log.tag.c_str(), log.message.c_str());
	file->Write(toPrint.c_str(), toPrint.size());
}


//-----------------------------------------------------------------------------------------------
// Callback for writing a log to the debugger output pane
//
static void WriteToDebugOutput(LogMessage_t log, void* fileptr)
{
	UNUSED(fileptr);
	std::string toPrint = Stringf("%s: %s\n", log.tag.c_str(), log.message.c_str());

#if defined( PLATFORM_WINDOWS )
	if( IsDebuggerAvailable() )
	{
		OutputDebugStringA(toPrint.c_str());
	}
#endif

	std::cout << toPrint.c_str();
}


//-----------------------------------------------------------------------------------------------
// Adds the text to the log with the tag "Log"
//
void LogPrintf(char const *format, ...)
{
	// Construct the string
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogPrintv(format, variableArgumentList);
	va_end(variableArgumentList);
}


//-----------------------------------------------------------------------------------------------
// Adds the text to the log with the tag "Log"
//
void LogPrintv(char const* format, va_list args)
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s(textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, args);	
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	LogMessage_t log;
	log.message	= std::string(textLiteral);
	log.tag = "LOG";

	LogSystem::AddLog(log);
}


//-----------------------------------------------------------------------------------------------
// Adds the given tag and text to the log system as a log
//
void LogTaggedPrintv(char const* tag, char const* format, va_list args)
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
void LogTaggedPrintf(char const* tag, char const *format, ...)
{
	// Construct the string
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTaggedPrintv(tag, format, variableArgumentList);
	va_end(variableArgumentList);
}


//-----------------------------------------------------------------------------------------------
// Enters a log with the tag "WARNING"
//
void LogWarningf(char const *format, ...)
{
	// Construct the string
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTaggedPrintv("WARNING", format, variableArgumentList);
	va_end(variableArgumentList);
}


//-----------------------------------------------------------------------------------------------
// Enters a log with the tag "ERROR"
//
void LogErrorf(char const *format, ...)
{
	// Construct the string
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTaggedPrintv("ERROR", format, variableArgumentList);
	va_end(variableArgumentList);
}


//////////////////////////////////////////////////////////////////////////
// Console Commands
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------
// Opens a large file and parses it, printing the line to the LogSystem as logs
//
void ThreadOpenBig(void* arguments)
{
	int i = *((int*) arguments);

	File* file = new File();
	
	const char* filepath = &(((const char*)arguments)[sizeof(int)]);
	bool success = file->Open(filepath, "r");

	if (!success)
	{
		ConsoleErrorf("Error: File \"%s\" couldn't be opened", filepath);
		return;
	}

	file->LoadFileToMemory();

	int count = 0;
	while (!file->IsAtEndOfFile())
	{
		count++;
		std::string line;
		unsigned int lineNumber = file->GetNextLine(line);
		LogPrintf("[%u:%u] %s", i, lineNumber, line.c_str());
	}

	ConsolePrintf(Rgba::GREEN, "Thread %i completed.", i);
}


//-----------------------------------------------------------------------------------------------
// Tests the threaded LogSystem by spinning up threads and having them all send logs to be logged
//
void Command_TestFlood(Command& cmd)
{
	ProfileScoped ps("flood");
	UNUSED(ps);

	// Get the thread count, default to 4
	int threadCount = 4;
	cmd.GetParam("c", threadCount, &threadCount);

	// Get the source file name
	std::string filepath;
	cmd.GetParam("f", filepath, &filepath);

	if (IsStringNullOrEmpty(filepath))
	{
		ConsoleErrorf("Error: No path specified");
		return;
	}

	// Get the arguments together
	unsigned char* args = (unsigned char*) malloc(sizeof(int) + sizeof(char) * (filepath.size() + 1)); // Add 1 to null terminate
	
	for (int i = 0; i < filepath.size() + 1; ++i)
	{
		int offset = i + 4; // Offset 4 to get around the thread count arg
		args[offset] = filepath.c_str()[i];
	}

	for (int i = 0; i < threadCount; ++i)
	{
		ProfileScoped pss(Stringf("%i thread creation.", i));
		UNUSED(pss);

		*((int*) args) = i; // Pass the thread id
		Thread::CreateAndDetach(ThreadOpenBig, args); // Create it
	}

	free(args); // Free up the argument after all threads are going, to be safe

	ConsolePrintf(Rgba::GREEN, "Flood test started for %i threads.", threadCount);
}


//-----------------------------------------------------------------------------------------------
// Tests the threaded LogSystem by creating a write and immediately flushing to disk
//
void Command_TestFlush(Command& cmd)
{
	std::string message = "This is a flush test.";
	cmd.GetParam("m", message, &message);

	LogMessage_t log;
	log.message = message;
	log.tag = "FLUSH TEST";

	LogSystem::AddLog(log);
	LogSystem::FlushLog();

	ASSERT_RECOVERABLE(false, Stringf("Flush log called, check the log file to ensure \"%s\" was written to \"Data/Logs/SystemLog.txt.\"", message.c_str()));
}


//-----------------------------------------------------------------------------------------------
// Sets all log hooks to show all tags
//
static void Command_ShowAllTags(Command& cmd)
{
	UNUSED(cmd);

	LogSystem::ShowAllTags();
	ConsolePrintf(Rgba::GREEN, "All tags enabled on LogSystem hooks");
}


//-----------------------------------------------------------------------------------------------
// Sets all hooks to hide all tags
//
static void Command_HideAllTags(Command& cmd)
{
	UNUSED(cmd);

	LogSystem::HideAllTags();
	ConsolePrintf(Rgba::GREEN, "All tags disabled on LogSystem hooks");
}