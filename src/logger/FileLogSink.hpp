#pragma once

#include "logger/ILogSink.hpp"

#include <fstream>
#include <string>

namespace logger {

/**
 * @brief Политика сброса буфера файла.
 */
enum class FlushPolicy {
    EveryRecord,
    Buffered
};

/**
 * @brief Запись строк журнала в текстовый файл.
 */
class FileLogSink final : public ILogSink {
public:
    /**
     * @param path Путь к файлу журнала. Файл создаётся, если отсутствует.
     * @param flushPolicy Когда сбрасывать буфер на диск.
     * @throw std::runtime_error если имя пустое или файл не удалось открыть.
     */
    explicit FileLogSink(std::string path,
                         FlushPolicy flushPolicy = FlushPolicy::EveryRecord);

    /**
     * @brief Дописывает строку в файл.
     * @throw std::runtime_error если запись не удалась.
     */
    void write(std::string_view record) override;

    /// @return Путь к файлу журнала.
    const std::string& path() const noexcept { return path_; }

private:
    std::string path_;
    FlushPolicy flushPolicy_;
    std::ofstream stream_;
};

}  // namespace logger
