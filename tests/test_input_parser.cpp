#include "Printers.hpp"
#include "TestFramework.hpp"

#include "InputParser.hpp"

#include <optional>
#include <string>

using app::parseInput;
using app::ParsedInput;
using logger::LogLevel;

TEST(УровеньОтделяетсяОтТекста) {
    const ParsedInput parsed = parseInput("ERROR диск переполнен");

    CHECK_EQ(parsed.level, std::optional<LogLevel>(LogLevel::Error));
    CHECK_EQ(parsed.text, std::string("диск переполнен"));
}

TEST(УровеньРаспознаётсяВЛюбомРегистре) {
    CHECK_EQ(parseInput("info сообщение").level,
             std::optional<LogLevel>(LogLevel::Info));
    CHECK_EQ(parseInput("WaRn сообщение").level,
             std::optional<LogLevel>(LogLevel::Warning));
}

TEST(БезУровняВсяСтрокаЯвляетсяТекстом) {
    const ParsedInput parsed = parseInput("просто сообщение");

    CHECK_EQ(parsed.level, std::optional<LogLevel>());
    CHECK_EQ(parsed.text, std::string("просто сообщение"));
}

TEST(ЛишниеПробелыОтбрасываются) {
    const ParsedInput parsed = parseInput("   WARNING    место кончается   ");

    CHECK_EQ(parsed.level, std::optional<LogLevel>(LogLevel::Warning));
    CHECK_EQ(parsed.text, std::string("место кончается"));
}

TEST(ПустаяСтрокаДаётПустойРезультат) {
    CHECK(parseInput("").empty());
    CHECK(parseInput("    ").empty());
    CHECK(parseInput("\t \n").empty());
}

TEST(ОдинокоеСловоУровняСчитаетсяТекстом) {
    const ParsedInput parsed = parseInput("INFO");

    CHECK_EQ(parsed.level, std::optional<LogLevel>());
    CHECK_EQ(parsed.text, std::string("INFO"));
}

TEST(ТекстНачинающийсяСоСловаУровняПоглощаетЕго) {
    const ParsedInput parsed = parseInput("error в модуле оплаты");

    CHECK_EQ(parsed.level, std::optional<LogLevel>(LogLevel::Error));
    CHECK_EQ(parsed.text, std::string("в модуле оплаты"));
}

TEST(ЭкранированиеСохраняетСтрокуДословно) {
    const ParsedInput parsed = parseInput("\\error в модуле оплаты");

    CHECK_EQ(parsed.level, std::optional<LogLevel>());
    CHECK_EQ(parsed.text, std::string("error в модуле оплаты"));
}

TEST(ЭкранированиеРаботаетИДляОбычногоТекста) {
    const ParsedInput parsed = parseInput("\\обычный текст");

    CHECK_EQ(parsed.level, std::optional<LogLevel>());
    CHECK_EQ(parsed.text, std::string("обычный текст"));
}

TEST(ПохожиеНаУровеньСловаНеПутаются) {
    const ParsedInput parsed = parseInput("errors выросли вдвое");

    CHECK_EQ(parsed.level, std::optional<LogLevel>());
    CHECK_EQ(parsed.text, std::string("errors выросли вдвое"));
}

TEST(ВнутренниеПробелыВТекстеСохраняются) {
    const ParsedInput parsed = parseInput("INFO шаг 1   шаг 2");

    CHECK_EQ(parsed.text, std::string("шаг 1   шаг 2"));
}
