#include "Application.hpp"

#include "InputParser.hpp"

#include <chrono>
#include <istream>
#include <ostream>
#include <string>
#include <utility>

namespace app {

Application::Application(const CliOptions& options, IMessageSink& sink)
    : options_(options),
      sink_(sink),
      currentLevel_(options.defaultLevel),
      commands_(sink_, currentLevel_) {}

void Application::printGreeting(std::ostream& output) const {
    output << "Журнал: " << options_.logFile << "\n"
           << "Уровень по умолчанию: "
           << logger::toString(options_.defaultLevel) << "\n\n"
           << inputSyntaxHelp() << "\nКоманды:\n"
           << CommandProcessor::helpText() << "\n";
}

void Application::run(std::istream& input, std::ostream& output) {
    printGreeting(output);

    std::string line;
    while (std::getline(input, line)) {
        const auto received = std::chrono::system_clock::now();

        const CommandProcessor::Result result = commands_.execute(line, output);
        if (result == CommandProcessor::Result::Quit) {
            return;
        }
        if (result == CommandProcessor::Result::Handled) {
            continue;
        }

        ParsedInput parsed = parseInput(line);
        if (parsed.empty()) {
            continue;
        }

        LogRequest request;
        request.text = std::move(parsed.text);
        request.level = parsed.level.value_or(currentLevel_);
        request.timestamp = received;

        if (!sink_.submit(std::move(request))) {
            output << "приём сообщений остановлен\n";
            return;
        }
    }
}

}  // namespace app
