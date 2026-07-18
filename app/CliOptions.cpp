#include "CliOptions.hpp"

#include "CommandProcessor.hpp"
#include "InputParser.hpp"

#include <string_view>

namespace app {
namespace {

bool isHelpFlag(std::string_view argument) noexcept {
    return argument == "-h" || argument == "--help";
}

}  // namespace

CliParseResult parseCommandLine(int argc, const char* const* argv) {
    CliParseResult result;

    for (int i = 1; i < argc; ++i) {
        if (isHelpFlag(argv[i])) {
            result.helpRequested = true;
            return result;
        }
    }

    if (argc < 2) {
        result.error = "не указано имя файла журнала";
        return result;
    }

    if (argc > 3) {
        result.error = "слишком много аргументов";
        return result;
    }

    CliOptions options;
    options.logFile = argv[1];

    if (options.logFile.empty()) {
        result.error = "имя файла журнала не может быть пустым";
        return result;
    }

    if (argc == 3) {
        const std::optional<logger::LogLevel> level =
            logger::parseLevel(argv[2]);

        if (!level) {
            result.error = std::string("неизвестный уровень важности: \"") +
                           argv[2] + "\"";
            return result;
        }
        options.defaultLevel = *level;
    }

    result.options = std::move(options);
    return result;
}

std::string usageText(const std::string& programName) {
    return "Использование: " + programName +
           " <файл журнала> [уровень по умолчанию]\n"
           "\n"
           "  файл журнала          путь к текстовому файлу; создаётся при "
           "отсутствии\n"
           "  уровень по умолчанию  INFO, WARNING или ERROR (по умолчанию "
           "INFO)\n"
           "\n" +
           inputSyntaxHelp() + "\nКоманды:\n" + CommandProcessor::helpText();
}

}  // namespace app
