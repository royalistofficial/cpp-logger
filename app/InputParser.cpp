#include "InputParser.hpp"

#include <cctype>

namespace app {
namespace {

constexpr char kEscapeChar = '\\';

bool isSpace(char c) noexcept {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::string_view trim(std::string_view text) noexcept {
    std::size_t begin = 0;
    while (begin < text.size() && isSpace(text[begin])) {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin && isSpace(text[end - 1])) {
        --end;
    }

    return text.substr(begin, end - begin);
}

struct SplitResult {
    std::string_view first;
    std::string_view rest;
};

SplitResult splitFirstWord(std::string_view text) noexcept {
    const std::size_t wordEnd = [&] {
        std::size_t i = 0;
        while (i < text.size() && !isSpace(text[i])) {
            ++i;
        }
        return i;
    }();

    return {text.substr(0, wordEnd), trim(text.substr(wordEnd))};
}

} 

ParsedInput parseInput(std::string_view line) {
    const std::string_view trimmed = trim(line);

    if (trimmed.empty()) {
        return {};
    }

    if (trimmed.front() == kEscapeChar) {
        return {std::string(trimmed.substr(1)), std::nullopt};
    }

    const SplitResult split = splitFirstWord(trimmed);
    const std::optional<logger::LogLevel> level =
        logger::parseLevel(split.first);

    if (level && !split.rest.empty()) {
        return {std::string(split.rest), level};
    }

    return {std::string(trimmed), std::nullopt};
}

} 