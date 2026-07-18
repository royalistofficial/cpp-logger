#pragma once

#include "logger/LogLevel.hpp"

#include <optional>
#include <string>

namespace app {

/**
 * @brief Параметры запуска приложения.
 */
struct CliOptions {
    std::string logFile;
    logger::LogLevel defaultLevel = logger::LogLevel::Info;
};

/**
 * @brief Результат разбора аргументов командной строки.
 */
struct CliParseResult {
    std::optional<CliOptions> options;
    std::string error;
    bool helpRequested = false;
};

/**
 * @brief Разбирает аргументы: <файл журнала> [уровень по умолчанию].
 */
CliParseResult parseCommandLine(int argc, const char* const* argv);

/// @return Текст справки по использованию.
std::string usageText(const std::string& programName);

}  // namespace app
