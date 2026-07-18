#include "logger/LogLevel.hpp"

#include <cstddef>

namespace logger {
namespace {

struct LevelName {
    std::string_view name;
    LogLevel level;
};

constexpr LevelName kLevelNames[] = {
    {"INFO", LogLevel::Info},
    {"WARNING", LogLevel::Warning},
    {"WARN", LogLevel::Warning},
    {"ERROR", LogLevel::Error},
    {"ERR", LogLevel::Error},
};

constexpr char toUpper(char c) noexcept {
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

/// Сравнение строк без учёта регистра.
bool equalsIgnoreCase(std::string_view lhs, std::string_view rhs) noexcept {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (toUpper(lhs[i]) != toUpper(rhs[i])) {
            return false;
        }
    }
    return true;
}

}

std::string_view toString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
    }
    return "UNKNOWN";
}

std::optional<LogLevel> parseLevel(std::string_view text) noexcept {
    for (const LevelName& entry : kLevelNames) {
        if (equalsIgnoreCase(text, entry.name)) {
            return entry.level;
        }
    }
    return std::nullopt;
}

}  