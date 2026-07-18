#pragma once

#include "logger/Export.hpp"
#include "logger/ILogFormatter.hpp"
#include "logger/ILogSink.hpp"
#include "logger/ILogger.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace logger {

/**
 * @brief Реализация журнала: фильтрация по уровню и потокобезопасная запись.
 *
 * Класс отвечает только за политику (порог, синхронизацию) и делегирует
 * представление записи форматтеру, а ввод-вывод — приёмнику. Заменив их,
 * можно писать в сокет или в другом формате, не трогая этот класс.
 */
class LOGGER_API Logger final : public ILogger {
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
    /// Форматирует и записывает без проверки порога.
    void emit(std::string_view message, LogLevel level,
              std::chrono::system_clock::time_point timestamp);

    std::unique_ptr<ILogSink> sink_;
    std::unique_ptr<ILogFormatter> formatter_;

    std::atomic<LogLevel> defaultLevel_;
    std::mutex sinkMutex_;
};

/**
 * @brief Создаёт журнал, пишущий в текстовый файл.
 * @param path Имя файла журнала. Файл создаётся, если отсутствует.
 * @param defaultLevel Начальный порог важности.
 * @throw std::runtime_error если файл не удалось открыть.
 */
LOGGER_API std::unique_ptr<ILogger> makeFileLogger(const std::string& path,
                                                   LogLevel defaultLevel);

}  // namespace logger
