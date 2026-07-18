#pragma once

#include "CliOptions.hpp"
#include "CommandProcessor.hpp"
#include "IMessageSink.hpp"

#include <iosfwd>

namespace app {

/**
 * @brief Цикл чтения консоли.
 *
 * Класс занят только разбором ввода: он не создаёт журнал, не владеет
 * потоками и не выводит итоги. Это делает композиционный корень (main),
 * а тестам достаточно подставить свой IMessageSink.
 */
class Application {
public:
    /**
     * @param options Параметры запуска (нужны для приветствия и начального
     *        уровня).
     * @param sink Приёмник сообщений. Должен пережить объект.
     */
    Application(const CliOptions& options, IMessageSink& sink);

    /**
     * @brief Читает строки, пока ввод не закончится или не придёт :quit.
     *
     * @param input Источник команд (обычно std::cin).
     * @param output Поток подсказок и ответов (обычно std::cout).
     */
    void run(std::istream& input, std::ostream& output);

    /// @return Уровень по умолчанию с учётом выполненных команд :level.
    logger::LogLevel currentLevel() const noexcept { return currentLevel_; }

private:
    void printGreeting(std::ostream& output) const;

    CliOptions options_;
    IMessageSink& sink_;
    logger::LogLevel currentLevel_;
    CommandProcessor commands_;
};

}  // namespace app
