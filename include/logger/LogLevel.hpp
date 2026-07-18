#pragma once

#include <optional>
#include <string_view>

namespace logger {
enum class LogLevel {
    Info = 0,
    Warning = 1,
    Error = 2
};

std::string_view toString(LogLevel level) noexcept;

std::optional<LogLevel> parseLevel(std::string_view text) noexcept;

}