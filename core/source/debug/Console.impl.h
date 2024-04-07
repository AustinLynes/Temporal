#pragma once 
#include <iomanip>

template<class Args>
inline void Console::PrintArgs(Args args)
{
	bool isBoolean = std::is_same<Args, bool>::value;
	bool isFloatingPoint = std::is_same<Args, float>::value || std::is_same<Args, double>::value;

	std::cout << 
		(isBoolean ? std::boolalpha : std::noboolalpha) <<
		(isFloatingPoint ? std::setprecision(3) : std::setprecision(0))
		<< args << " ";
}


template<class ...Args>
inline void Console::Info(Args && ...args)
{
	PrintTimeStamp();
	PrintColored("[INFO]", LogSeverity::_INFO_);
	int dummy[] = { 0, ((void)PrintArgs(std::forward<Args>(args)), 0)... };
	std::cout << std::endl;
}

template<class ...Args>
inline void Console::Success(Args && ...args)
{
	PrintTimeStamp();
	PrintColored("[SUCCESS]", LogSeverity::_SUCCESS_);
	int dummy[] = { 0, ((void)PrintArgs(std::forward<Args>(args)), 0)... };
	std::cout << std::endl;
}

template<class... Args>
void Console::Log(Args&&... args) {
	PrintTimeStamp();
	PrintColored("[LOG]", LogSeverity::_LOG_);
	int dummy[] = { 0, ((void)PrintArgs(std::forward<Args>(args)), 0)... };
	std::cout << std::endl;
}

template<class ...Args>
inline void Console::Warn(Args && ...args)
{
	PrintTimeStamp();
	PrintColored("[WARN]", LogSeverity::_WARN_);
	int dummy[] = { 0, ((void)PrintArgs(std::forward<Args>(args)), 0)... };
	std::cout << std::endl;
}

template<class ...Args>
inline void Console::Error(Args && ...args)
{
	PrintTimeStamp();
	PrintColored("[ERROR]", LogSeverity::_ERROR_);
	int dummy[] = { 0, ((void)PrintArgs(std::forward<Args>(args)), 0)... };
	std::cout << std::endl;
}

template<class ...Args>
inline void Console::Fatal(Args && ...args)
{
	PrintTimeStamp();
	PrintColored("[FATAL]", LogSeverity::_FATAL_);
	int dummy[] = { 0, ((void)PrintArgs(std::forward<Args>(args)), 0)... };
	std::cout << std::endl;
}
