#pragma once

#include "TestFramework.hpp"

#include "logger/LogLevel.hpp"

#include <optional>
#include <string>

namespace testing {

inline std::string describe(logger::LogLevel level) {
    return std::string(logger::toString(level));
}

inline std::string describe(const std::optional<logger::LogLevel>& level) {
    return level ? describe(*level) : std::string("nullopt");
}

}
