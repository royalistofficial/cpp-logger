#include "Application.hpp"

#include "InputParser.hpp"
#include "LogWriterThread.hpp"

#include "logger/Logger.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace app {
namespace {

constexpr char kQuitCommand[] = ":quit";
constexpr char kLevelCommand[] = ":level";

void printGreeting(std::ostream& output, const CliOptions& options) {
    output << "Журнал: " << options.logFile << "\n"
           << "Уровень по умолчанию: " << logger::toString(options.defaultLevel)
           << "\n"
           << "Ввод: \"<уровень> <текст>\" либо просто \"<текст>\".\n"
           << "Команды: " << kLevelCommand << " <уровень> — сменить порог, "
           << kQuitCommand << " или Ctrl+D — выход.\n\n";
}

enum class CommandResult {
    NotACommand,
    Handled,
    Quit
};

CommandResult handleCommand(const std::string& line, LogQueue& queue,
                            logger::LogLevel& currentLevel,
                            std::ostream& output) {
    if (line == kQuitCommand) {
        return CommandResult::Quit;
    }

    const std::string prefix = std::string(kLevelCommand) + " ";
    if (line.rfind(prefix, 0) != 0) {
        return CommandResult::NotACommand;
    }

    const std::string argument = line.substr(prefix.size());
    if (const std::optional<logger::LogLevel> level =
            logger::parseLevel(argument)) {
        currentLevel = *level;
        queue.push(SetLevelCommand{*level});
        output << "уровень по умолчанию: " << logger::toString(*level) << "\n";
    } else {
        output << "неизвестный уровень: " << argument << "\n";
    }
    return CommandResult::Handled;
}

} 

Application::Application(CliOptions options) : options_(std::move(options)) {}

int Application::run(std::istream& input, std::ostream& output) {
    std::unique_ptr<logger::ILogger> log;

    try {
        log = logger::makeFileLogger(options_.logFile, options_.defaultLevel);
    } catch (const std::exception& error) {
        std::cerr << "не удалось открыть журнал: " << error.what() << "\n";
        return 1;
    }

    printGreeting(output, options_);

    LogQueue queue;
    LogWriterThread writer(queue, *log);

    logger::LogLevel currentLevel = options_.defaultLevel;

    std::string line;
    while (std::getline(input, line)) {
        const auto received = std::chrono::system_clock::now();

        switch (handleCommand(line, queue, currentLevel, output)) {
            case CommandResult::Quit:
                queue.close();
                writer.join();
                output << "обработано сообщений: " << writer.processed()
                       << "\n";
                return writer.failed() == 0 ? 0 : 1;
            case CommandResult::Handled:
                continue;
            case CommandResult::NotACommand:
                break;
        }

        const ParsedInput parsed = parseInput(line);
        if (parsed.empty()) {
            continue; 
        }

        LogRequest request;
        request.text = parsed.text;
        request.level = parsed.level.value_or(currentLevel);
        request.timestamp = received;

        queue.push(std::move(request));
    }

    queue.close();
    writer.join();

    output << "\nобработано сообщений: " << writer.processed() << "\n";
    if (writer.failed() != 0) {
        output << "потеряно из-за ошибок: " << writer.failed() << "\n";
        return 1;
    }
    return 0;
}

}  // namespace app
