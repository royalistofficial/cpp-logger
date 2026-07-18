#pragma once

#include "IMessageSink.hpp"

#include "logger/LogLevel.hpp"

#include <array>
#include <iosfwd>
#include <string>
#include <string_view>

namespace app {

/// Префикс, отличающий команду от обычного сообщения.
inline constexpr char kCommandPrefix = ':';

/**
 * @brief Разбор и выполнение команд управления.
 */
class CommandProcessor {
public:
    enum class Result {
        NotACommand,
        Handled, 
        Quit 
    };

    /**
     * @param sink Приёмник, которому передаются команды.
     * @param currentLevel Уровень по умолчанию на стороне приложения.\
     */
    CommandProcessor(IMessageSink& sink, logger::LogLevel& currentLevel);

    /**
     * @brief Выполняет строку, если она является командой.
     * @param output Поток для ответов пользователю.
     */
    Result execute(std::string_view line, std::ostream& output);

    /// @return Многострочное описание всех команд.
    static std::string helpText();

private:
    using Handler = Result (CommandProcessor::*)(std::string_view,
                                                 std::ostream&);

    struct Command {
        std::string_view name;
        std::string_view argument;
        std::string_view description;
        Handler handler;
    };

    Result doQuit(std::string_view argument, std::ostream& output);
    Result doLevel(std::string_view argument, std::ostream& output);
    Result doHelp(std::string_view argument, std::ostream& output);

    static const std::array<Command, 3>& commands();

    IMessageSink& sink_;
    logger::LogLevel& currentLevel_;
};

}  // namespace app
