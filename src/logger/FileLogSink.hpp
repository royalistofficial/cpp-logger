#pragma once

#include "logger/ILogSink.hpp"

#include <fstream>
#include <string>

namespace logger {

/**
 * @brief Запись строк журнала в текстовый файл.
 *
 * Файл открывается в режиме дописывания в конструкторе и закрывается в
 * деструкторе. Класс не синхронизирует доступ: за это отвечает вызывающая
 * сторона (класс Logger).
 */
class FileLogSink final : public ILogSink {
public:
    /**
     * @param path Путь к файлу журнала. Файл создаётся, если отсутствует.
     * @throw std::runtime_error если файл не удалось открыть.
     */
    explicit FileLogSink(std::string path);

    /**
     * @brief Дописывает строку в файл и сбрасывает буфер.
     * @throw std::runtime_error если запись не удалась.
     *
     * Сброс буфера после каждой записи гарантирует, что сообщения не
     * потеряются при аварийном завершении программы, и что об ошибке станет
     * известно сразу, а не при закрытии файла.
     */
    void write(std::string_view record) override;

    /// @return Путь к файлу журнала.
    const std::string& path() const noexcept { return path_; }

private:
    std::string path_;
    std::ofstream stream_;
};

}  // namespace logger
