#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace Time {

    /// <summary>
    /// Get Current TimeStamp
    /// </summary>
    /// <param name="format">
    /// Y -> Year, m -> month, d -> day
    /// H -> Hour, M -> Minute, S -> Second
    /// </param>
    /// <returns></returns>
    static std::string TimeStamp(const char* format = "%Y-%m-%d %H:%M:%S", bool add_precision=true) {
        // Get current system time
        auto now = std::chrono::system_clock::now();

        // Convert to time_t (seconds since epoch)
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        // Convert time_t to struct tm with local timezone
        std::tm* localTime = std::localtime(&currentTime);
       
        // Format the timestamp as a string
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), format, localTime);

        // Append milliseconds to the buffer
        std::string result(buffer);

        if (add_precision) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            result += ".";
            result += std::to_string(ms.count());
        }

        return result;
    }

}