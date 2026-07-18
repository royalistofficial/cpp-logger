#pragma once

#include "logger/ILogFormatter.hpp"
#include "logger/ILogger.hpp"
#include "logger/ILogSink.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace logger {

/**
 * @brief Реализация журнала: фильтрация по уровню и потокобезопасная запись.
 */
class Logger final : public ILogger {
public:
    /**
     * @param sink Приёмник готовых строк. Не может быть nullptr.
     * @param formatter Форматтер записей. Не может быть nullptr.
     * @param defaultLevel Начальный порог важности.
     * @throw std::invalid_argument если sink или formatter равны nullptr.
     */
    Logger(std::unique_ptr<ILogSink> sink,
           std::unique_ptr<ILogFormatter> formatter, LogLevel defaultLevel);

    void log(std::string_view message, LogLevel level) override;
    void log(std::string_view message) override;
    void log(std::string_view message, LogLevel level,
             std::chrono::system_clock::time_point timestamp) override;

    void setDefaultLevel(LogLevel level) noexcept override;
    LogLevel defaultLevel() const noexcept override;

private:
    std::unique_ptr<ILogSink> sink_;
    std::unique_ptr<ILogFormatter> formatter_;

    std::atomic<LogLevel> defaultLevel_;

    mutable std::mutex sinkMutex_;
};

/**
 * @brief Создаёт журнал, пишущий в текстовый файл.
 * @param path Имя файла журнала. Файл создаётся, если отсутствует.
 * @param defaultLevel Начальный порог важности.
 * @throw std::runtime_error если файл не удалось открыть.
 */
std::unique_ptr<ILogger> makeFileLogger(const std::string& path,
                                        LogLevel defaultLevel);

}
