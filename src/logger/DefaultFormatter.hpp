#pragma once

#include "logger/ILogFormatter.hpp"

namespace logger {

/**
 * @brief Формат по умолчанию: [ГГГГ-ММ-ДД ЧЧ:ММ:СС.мсс] [УРОВЕНЬ] текст
 */
class DefaultFormatter final : public ILogFormatter {
public:
    std::string format(const LogRecord& record) const override;
};

}  // namespace logger
