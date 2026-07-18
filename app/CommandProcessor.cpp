#include "CommandProcessor.hpp"

#include <cctype>
#include <optional>
#include <ostream>

namespace app {
namespace {

bool isSpace(char c) noexcept {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::string_view trim(std::string_view text) noexcept {
    std::size_t begin = 0;
    while (begin < text.size() && isSpace(text[begin])) {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin && isSpace(text[end - 1])) {
        --end;
    }

    return text.substr(begin, end - begin);
}

}  // namespace

CommandProcessor::CommandProcessor(IMessageSink& sink,
                                   logger::LogLevel& currentLevel)
    : sink_(sink), currentLevel_(currentLevel) {}

const std::array<CommandProcessor::Command, 3>& CommandProcessor::commands() {
    static const std::array<Command, 3> kCommands = {{
        {":quit", "", "завершить работу (то же делает Ctrl+D)",
         &CommandProcessor::doQuit},
        {":level", " <уровень>", "сменить уровень по умолчанию",
         &CommandProcessor::doLevel},
        {":help", "", "показать эту справку", &CommandProcessor::doHelp},
    }};
    return kCommands;
}

CommandProcessor::Result CommandProcessor::execute(std::string_view line,
                                                   std::ostream& output) {
    const std::string_view trimmed = trim(line);

    if (trimmed.empty() || trimmed.front() != kCommandPrefix) {
        return Result::NotACommand;
    }

    std::size_t nameEnd = 0;
    while (nameEnd < trimmed.size() && !isSpace(trimmed[nameEnd])) {
        ++nameEnd;
    }

    const std::string_view name = trimmed.substr(0, nameEnd);
    const std::string_view argument = trim(trimmed.substr(nameEnd));

    for (const Command& command : commands()) {
        if (command.name == name) {
            return (this->*command.handler)(argument, output);
        }
    }

    // Опечатка в команде не должна незаметно попасть в журнал как сообщение.
    output << "неизвестная команда: " << name << "\n"
           << "доступные команды:\n"
           << helpText();
    return Result::Handled;
}

CommandProcessor::Result CommandProcessor::doQuit(std::string_view,
                                                  std::ostream&) {
    return Result::Quit;
}

CommandProcessor::Result CommandProcessor::doLevel(std::string_view argument,
                                                   std::ostream& output) {
    if (argument.empty()) {
        output << "команда :level требует уровень, например: :level WARNING\n";
        return Result::Handled;
    }

    const std::optional<logger::LogLevel> level = logger::parseLevel(argument);
    if (!level) {
        output << "неизвестный уровень: " << argument << "\n";
        return Result::Handled;
    }

    currentLevel_ = *level;
    sink_.setDefaultLevel(*level);
    output << "уровень по умолчанию: " << logger::toString(*level) << "\n";
    return Result::Handled;
}

CommandProcessor::Result CommandProcessor::doHelp(std::string_view,
                                                  std::ostream& output) {
    output << helpText();
    return Result::Handled;
}

std::string CommandProcessor::helpText() {
    std::string text;
    for (const Command& command : commands()) {
        text += "  ";
        text += command.name;
        text += command.argument;
        text += " — ";
        text += command.description;
        text += '\n';
    }
    return text;
}

}  // namespace app
