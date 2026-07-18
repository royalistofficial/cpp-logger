#include "Printers.hpp"
#include "TestFramework.hpp"

#include "logger/DefaultFormatter.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <string>

using logger::DefaultFormatter;
using logger::LogLevel;
using logger::LogRecord;

namespace {

std::chrono::system_clock::time_point at(std::time_t seconds, int millis) {
    return std::chrono::system_clock::time_point(
        std::chrono::seconds(seconds) + std::chrono::milliseconds(millis));
}

LogRecord makeRecord(std::string text, LogLevel level,
                     std::chrono::system_clock::time_point tp) {
    LogRecord record;
    record.text = std::move(text);
    record.level = level;
    record.timestamp = tp;
    return record;
}

}  // namespace

TEST(ФорматСоответствуетОбразцу) {
    const DefaultFormatter formatter;
    const std::string line =
        formatter.format(makeRecord("привет", LogLevel::Warning, at(0, 0)));

    // [ГГГГ-ММ-ДД ЧЧ:ММ:СС.мсс] [УРОВЕНЬ] текст
    CHECK_EQ(line.front(), '[');
    CHECK(line.find("] [WARNING] привет\n") != std::string::npos);
    CHECK_EQ(line.substr(5, 1), std::string("-"));
    CHECK_EQ(line.substr(8, 1), std::string("-"));
    CHECK_EQ(line.substr(24, 1), std::string("]"));
}

TEST(МиллисекундыДополняютсяНулями) {
    const DefaultFormatter formatter;
    const std::string line =
        formatter.format(makeRecord("x", LogLevel::Info, at(0, 7)));

    CHECK(line.find(".007]") != std::string::npos);
}

TEST(ВремяДоЭпохиНеЛомаетФормат) {
    const DefaultFormatter formatter;
    // -1.5 секунды от эпохи: усечение к нулю дало бы ".-500"
    const std::string line =
        formatter.format(makeRecord("x", LogLevel::Info, at(-2, 500)));

    CHECK(line.find(".500]") != std::string::npos);
    CHECK(line.find("-500") == std::string::npos);
}

TEST(ПереводСтрокиВТекстеЭкранируется) {
    const DefaultFormatter formatter;
    const std::string line = formatter.format(
        makeRecord("первая\nвторая", LogLevel::Error, at(0, 0)));

    // Ровно один перевод строки — завершающий: одна запись = одна строка.
    CHECK_EQ(std::count(line.begin(), line.end(), '\n'),
             static_cast<std::ptrdiff_t>(1));
    CHECK(line.find("первая\\nвторая") != std::string::npos);
}

TEST(ВозвратКареткиЭкранируется) {
    const DefaultFormatter formatter;
    const std::string line =
        formatter.format(makeRecord("a\rb", LogLevel::Info, at(0, 0)));

    CHECK(line.find("a\\rb") != std::string::npos);
}

TEST(ПустойТекстНеЛомаетФормат) {
    const DefaultFormatter formatter;
    const std::string line =
        formatter.format(makeRecord("", LogLevel::Info, at(0, 0)));

    CHECK(line.find("[INFO] \n") != std::string::npos);
}

TEST(КаждаяЗаписьЗаканчиваетсяПереводомСтроки) {
    const DefaultFormatter formatter;
    for (const LogLevel level :
         {LogLevel::Info, LogLevel::Warning, LogLevel::Error}) {
        const std::string line =
            formatter.format(makeRecord("текст", level, at(1000, 1)));
        CHECK_EQ(line.back(), '\n');
        CHECK(line.find(std::string(logger::toString(level))) !=
              std::string::npos);
    }
}
