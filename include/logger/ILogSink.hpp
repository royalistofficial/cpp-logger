#pragma once

#include "logger/Export.hpp"

#include <string_view>

namespace logger {

/**
 * @brief Приёмник готовых строк журнала.
 *
 * Реализация не обязана быть потокобезопасной: доступ к ней сериализует
 * Logger.
 */
class LOGGER_API ILogSink {
public:
    virtual ~ILogSink() = default;

    ILogSink(const ILogSink&) = delete;
    ILogSink& operator=(const ILogSink&) = delete;

    /**
     * @brief Записывает готовую строку.
     * @throw std::runtime_error при любой ошибке ввода-вывода.
     */
    virtual void write(std::string_view record) = 0;

protected:
    ILogSink() = default;
};

}  // namespace logger
