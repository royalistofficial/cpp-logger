#include "logger/DefaultFormatter.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>

namespace logger {
namespace {

struct SplitTime {
    std::time_t seconds;
    int milliseconds;  
};

SplitTime split(std::chrono::system_clock::time_point tp) noexcept {
    using namespace std::chrono;

    const auto sinceEpoch = tp.time_since_epoch();
    const auto wholeSeconds = floor<seconds>(sinceEpoch);
    const auto rest = duration_cast<milliseconds>(sinceEpoch - wholeSeconds);

    return {static_cast<std::time_t>(wholeSeconds.count()),
            static_cast<int>(rest.count())};
}

void appendEscaped(std::string& out, std::string_view text) {
    out.reserve(out.size() + text.size());

    for (const char c : text) {
        switch (c) {
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            default:
                out += c;
                break;
        }
    }
}

}  // namespace

std::string DefaultFormatter::format(const LogRecord& record) const {
    const SplitTime time = split(record.timestamp);

    std::tm local{};
    if (localtime_r(&time.seconds, &local) == nullptr) {
        local = std::tm{};
    }
\
    char header[32];
    const int written =
        std::snprintf(header, sizeof(header), "[%04d-%02d-%02d %02d:%02d:%02d.%03d] [",
                      local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                      local.tm_hour, local.tm_min, local.tm_sec,
                      time.milliseconds);

    std::string line;
    if (written > 0) {
        line.assign(header, static_cast<std::size_t>(written));
    }

    line += toString(record.level);
    line += "] ";
    appendEscaped(line, record.text);
    line += '\n';

    return line;
}

}  // namespace logger
