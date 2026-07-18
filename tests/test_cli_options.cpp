#include "Printers.hpp"
#include "TestFramework.hpp"

#include "CliOptions.hpp"

#include <string>
#include <vector>

using app::CliParseResult;
using app::parseCommandLine;
using logger::LogLevel;

namespace {
CliParseResult parse(std::vector<const char*> arguments) {
    return parseCommandLine(static_cast<int>(arguments.size()),
                            arguments.data());
}

}

TEST(ФайлИУровеньРазбираются) {
    const CliParseResult result = parse({"logger_app", "app.log", "warning"});

    CHECK(result.options.has_value());
    if (result.options) {
        CHECK_EQ(result.options->logFile, std::string("app.log"));
        CHECK_EQ(result.options->defaultLevel, LogLevel::Warning);
    }
}

TEST(БезУровняИспользуетсяINFO) {
    const CliParseResult result = parse({"logger_app", "app.log"});

    CHECK(result.options.has_value());
    if (result.options) {
        CHECK_EQ(result.options->defaultLevel, LogLevel::Info);
    }
}

TEST(БезАргументовОшибка) {
    const CliParseResult result = parse({"logger_app"});

    CHECK(!result.options.has_value());
    CHECK(!result.error.empty());
}

TEST(НеизвестныйУровеньОтвергается) {
    const CliParseResult result = parse({"logger_app", "app.log", "trace"});

    CHECK(!result.options.has_value());
    CHECK(result.error.find("trace") != std::string::npos);
}

TEST(ЛишниеАргументыОтвергаются) {
    const CliParseResult result =
        parse({"logger_app", "app.log", "info", "лишнее"});

    CHECK(!result.options.has_value());
    CHECK(!result.error.empty());
}

TEST(ПустоеИмяФайлаОтвергается) {
    const CliParseResult result = parse({"logger_app", ""});

    CHECK(!result.options.has_value());
    CHECK(!result.error.empty());
}

TEST(ФлагСправкиРаспознаётся) {
    CHECK(parse({"logger_app", "--help"}).helpRequested);
    CHECK(parse({"logger_app", "-h"}).helpRequested);
    const CliParseResult result = parse({"logger_app", "app.log", "--help"});
    CHECK(result.helpRequested);
    CHECK(result.error.empty());
}

TEST(СправкаНазываетПрограммуИУровни) {
    const std::string text = app::usageText("logger_app");

    CHECK(text.find("logger_app") != std::string::npos);
    CHECK(text.find("INFO") != std::string::npos);
    CHECK(text.find("ERROR") != std::string::npos);
}
