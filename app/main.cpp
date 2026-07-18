#include "Application.hpp"
#include "AsyncLogWriter.hpp"
#include "CliOptions.hpp"

#include "logger/Logger.hpp"

#include <exception>
#include <iostream>
#include <memory>

namespace {

enum ExitCode : int {
    kSuccess = 0,
    kUsageError = 2,
    kRuntimeError = 1
};

void printStats(std::ostream& output, const app::WriterStats& stats) {
    output << "\nпередано в журнал: " << stats.processed
           << " (часть могла быть отброшена фильтром уровня)\n";
    if (stats.failed != 0) {
        output << "потеряно из-за ошибок записи: " << stats.failed << "\n";
    }
    if (stats.dropped != 0) {
        output << "не принято после остановки: " << stats.dropped << "\n";
    }
}

}  // namespace

/**
 * @brief Композиционный корень: разбирает аргументы и связывает компоненты.
 */
int main(int argc, char** argv) {
    const std::string programName = (argc > 0 && argv[0] != nullptr)
                                        ? argv[0]
                                        : "logger_app";

    const app::CliParseResult parsed = app::parseCommandLine(argc, argv);

    if (parsed.helpRequested) {
        std::cout << app::usageText(programName);
        return kSuccess;
    }

    if (!parsed.options) {
        std::cerr << "ошибка: " << parsed.error << "\n\n"
                  << app::usageText(programName);
        return kUsageError;
    }

    const app::CliOptions& options = *parsed.options;

    std::unique_ptr<logger::ILogger> log;
    try {
        log = logger::makeFileLogger(options.logFile, options.defaultLevel);
    } catch (const std::exception& error) {
        std::cerr << "не удалось открыть журнал: " << error.what() << "\n";
        return kRuntimeError;
    }

    app::WriterStats stats;
    {
        app::AsyncLogWriter writer(*log, std::cerr);
        app::Application application(options, writer);

        application.run(std::cin, std::cout);

        stats = writer.finish();
    }

    printStats(std::cout, stats);
    return stats.failed == 0 ? kSuccess : kRuntimeError;
}
