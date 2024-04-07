#pragma once



enum class LogSeverity {
	_INFO_,
	_SUCCESS_,
	_LOG_,
	_WARN_,
	_ERROR_,
	_FATAL_
};

#include <iostream>
#include <iomanip>
#include <string>
#include <utility>

class Console {


public:
	template<class... Args>
	static void Info(Args&&... args);

	template<class... Args>
	static void Success(Args&&... args);

	template<class... Args>
	static void Log(Args&&... args);

	template<class... Args>
	static void Warn(Args&&... args);

	template<class... Args>
	static void Error(Args&&...args);

	template<class... Args>
	static void Fatal(Args&&...args);



private:

	static void PrintTimeStamp();
	static void PrintColored(const char* log, LogSeverity severity);
	
	template<class Args>
	static void PrintArgs(Args args);
	static void PrintArgs(float args);
	static void PrintArgs(double args);


};

#include "Console.impl.h"