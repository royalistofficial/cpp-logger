#include "logger/DefaultFormatter.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace logger {
namespace {

struct SplitTime {
    std::time_t seconds;
    long milliseconds;
};

SplitTime split(std::chrono::system_clock::time_point tp) {
    using namespace std::chrono;

    const auto since_epoch = tp.time_since_epoch();
    const auto whole_seconds = duration_cast<seconds>(since_epoch);
    const auto rest = duration_cast<milliseconds>(since_epoch - whole_seconds);

    return {static_cast<std::time_t>(whole_seconds.count()),
            static_cast<long>(rest.count())};
}

} 

std::string DefaultFormatter::format(const LogRecord& record) const {
    const SplitTime time = split(record.timestamp);

    std::tm local{};
    localtime_r(&time.seconds, &local);

    std::ostringstream out;
    out << '[' << std::put_time(&local, "%Y-%m-%d %H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << time.milliseconds << "] ["
        << toString(record.level) << "] " << record.text << '\n';

    return out.str();
}

} 
