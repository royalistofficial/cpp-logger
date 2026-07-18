#pragma once

#include <string_view>

namespace logger {

/**
 * @brief Приёмник готовых строк журнала.
 */
class ILogSink {
public:
    virtual ~ILogSink() = default;

    /**
     * @brief Записывает готовую строку.
     * @throw std::runtime_error при любой ошибке ввода-вывода.
     */
    virtual void write(std::string_view record) = 0;
};

}
