#pragma once

#include "logger/Export.hpp"

#include <optional>
#include <string_view>

namespace logger {

/**
 * @brief Уровень важности сообщения.
 */
enum class LogLevel {
    Info = 0,
    Warning = 1,
    Error = 2
};

/// @return Каноническое имя уровня в верхнем регистре.
LOGGER_API std::string_view toString(LogLevel level) noexcept;

/**
 * @brief Разбирает имя уровня без учёта регистра.
 * @return Уровень либо std::nullopt, если имя не распознано.
 */
LOGGER_API std::optional<LogLevel> parseLevel(std::string_view text) noexcept;

}  // namespace logger
