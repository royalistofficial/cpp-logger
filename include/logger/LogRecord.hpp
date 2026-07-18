#pragma once

#include "logger/LogLevel.hpp"

#include <chrono>
#include <string>

namespace logger {

/**
 * @brief Одно сообщение журнала со всеми сопутствующими данными.
 */
struct LogRecord {
    std::string text;
    LogLevel level = LogLevel::Info;
    std::chrono::system_clock::time_point timestamp;
};

}
