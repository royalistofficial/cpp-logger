#include "logger/Logger.hpp"

#include "logger/DefaultFormatter.hpp"
#include "logger/FileLogSink.hpp"

#include <stdexcept>
#include <utility>

namespace logger {

Logger::Logger(std::unique_ptr<ILogSink> sink,
               std::unique_ptr<ILogFormatter> formatter, LogLevel defaultLevel)
    : sink_(std::move(sink)),
      formatter_(std::move(formatter)),
      defaultLevel_(defaultLevel) {
    if (!sink_) {
        throw std::invalid_argument("Logger: приёмник не задан");
    }
    if (!formatter_) {
        throw std::invalid_argument("Logger: форматтер не задан");
    }
}

void Logger::log(std::string_view message, LogLevel level,
                 std::chrono::system_clock::time_point timestamp) {
    // Порог читается один раз: параллельная смена уровня не должна приводить
    // к разным решениям внутри одного вызова.
    if (level < defaultLevel_.load(std::memory_order_relaxed)) {
        return;
    }

    LogRecord record;
    record.text = std::string(message);
    record.level = level;
    record.timestamp = timestamp;

    // Форматирование вынесено за пределы блокировки: оно не трогает общих
    // данных, а держать мьютекс дольше необходимого незачем.
    const std::string line = formatter_->format(record);

    const std::lock_guard<std::mutex> lock(sinkMutex_);
    sink_->write(line);
}

void Logger::log(std::string_view message, LogLevel level) {
    log(message, level, std::chrono::system_clock::now());
}

void Logger::log(std::string_view message) {
    log(message, defaultLevel_.load(std::memory_order_relaxed));
}

void Logger::setDefaultLevel(LogLevel level) noexcept {
    defaultLevel_.store(level, std::memory_order_relaxed);
}

LogLevel Logger::defaultLevel() const noexcept {
    return defaultLevel_.load(std::memory_order_relaxed);
}

std::unique_ptr<ILogger> makeFileLogger(const std::string& path,
                                        LogLevel defaultLevel) {
    return std::make_unique<Logger>(std::make_unique<FileLogSink>(path),
                                    std::make_unique<DefaultFormatter>(),
                                    defaultLevel);
}

}  // namespace logger
