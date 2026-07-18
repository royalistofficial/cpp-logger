#pragma once

#include "logger/Export.hpp"
#include "logger/LogRecord.hpp"

#include <string>

namespace logger {

/**
 * @brief Преобразование записи журнала в строку.
 */
class LOGGER_API ILogFormatter {
public:
    virtual ~ILogFormatter() = default;

    ILogFormatter(const ILogFormatter&) = delete;
    ILogFormatter& operator=(const ILogFormatter&) = delete;

    /// @return Готовая строка, включая завершающий перевод строки.
    virtual std::string format(const LogRecord& record) const = 0;

protected:
    ILogFormatter() = default;
};

}  // namespace logger
