#include "Console.h"

#include <iostream>
#include <time/util.h>

const std::string BLACK = "\033[0;30m";
const std::string RED = "\033[0;31m";
const std::string GREEN = "\033[0;32m";
const std::string YELLOW = "\033[0;33m";
const std::string BLUE = "\033[0;34m";
const std::string MAGENTA = "\033[0;35m";
const std::string CYAN = "\033[0;36m";
const std::string WHITE = "\033[0;37m";


const std::string LIGHT_BLACK = "\033[0;90m";
const std::string LIGHT_RED = "\033[0;91m";
const std::string LIGHT_GREEN = "\033[0;92m";
const std::string LIGHT_YELLOW = "\033[0;93m";
const std::string LIGHT_BLUE = "\033[0;94m";
const std::string LIGHT_MAGENTA = "\033[0;95m";
const std::string LIGHT_CYAN = "\033[0;96m";
const std::string LIGHT_WHITE = "\033[0;97m";

void Console::PrintColored(const char* log, LogSeverity severity)
{
	const char* color_code = nullptr;

	switch (severity)
	{
	case LogSeverity::_INFO_:
		color_code = WHITE.c_str();
		break;
	case LogSeverity::_SUCCESS_:
		color_code = GREEN.c_str();
		break;
	case LogSeverity::_LOG_:
		color_code = BLUE.c_str();
		break;
	case LogSeverity::_WARN_:
		color_code = YELLOW.c_str();
		break;
	case LogSeverity::_ERROR_:
		color_code = RED.c_str();
		break;
	case LogSeverity::_FATAL_:
		color_code = RED.c_str();
		break;
	}

	std::cout << color_code << log << "\033[0m -- ";
}

void Console::PrintArgs(float args) {
	std::cout << std::fixed << std::setprecision(12) << args << " ";
}

void Console::PrintArgs(double args) {
	std::cout << std::fixed << std::setprecision(12) << args << " ";
}

void Console::PrintTimeStamp() {
	auto stamp = Time::TimeStamp();
	std::cout << stamp << " ";
}
