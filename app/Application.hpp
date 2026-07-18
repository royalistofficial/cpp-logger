#pragma once

#include "CliOptions.hpp"

#include <iosfwd>

namespace app {

/**
 * @brief Консольное приложение: читает сообщения и передаёт их в журнал.
 */
class Application {
public:
    explicit Application(CliOptions options);

    /**
     * @brief Запускает цикл обработки ввода.
     *
     * @param input Источник команд (обычно std::cin).
     * @param output Поток для подсказок и итогов (обычно std::cout).
     * @return Код возврата процесса: 0 при успехе.
     */
    int run(std::istream& input, std::ostream& output);

private:
    CliOptions options_;
};

} 
