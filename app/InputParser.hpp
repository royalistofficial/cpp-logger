#pragma once

#include "logger/LogLevel.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace app {

inline constexpr char kEscapeChar = '\\';

/**
 * @brief Результат разбора строки, введённой пользователем.
 */
struct ParsedInput {
    std::string text;
    std::optional<logger::LogLevel> level;

    /// @return true, если строка не содержит сообщения (пустая или пробелы).
    bool empty() const noexcept { return text.empty(); }
};

/**
 * @brief Разбирает строку вида "[уровень] текст сообщения".
 */
ParsedInput parseInput(std::string_view line);

/// @return Описание синтаксиса ввода. Общий текст для справки и приветствия.
std::string inputSyntaxHelp();

}  // namespace app
