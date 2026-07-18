#pragma once

#include "logger/LogRecord.hpp"

#include <string>

namespace logger {

/**
 * @brief Преобразование записи журнала в строку.
 */
class ILogFormatter {
public:
    virtual ~ILogFormatter() = default;

    /// @return Готовая строка, включая завершающий перевод строки.
    virtual std::string format(const LogRecord& record) const = 0;
};

}
