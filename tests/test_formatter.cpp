#include "TestFramework.hpp"

#include "logger/DefaultFormatter.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>

using logger::DefaultFormatter;
using logger::LogLevel;
using logger::LogRecord;

namespace {

void useUtc() {
    setenv("TZ", "UTC", 3);
    tzset();
}

std::chrono::system_clock::time_point makeTime(long long seconds,
                                               int milliseconds) {
    using namespace std::chrono;
    return system_clock::time_point(std::chrono::seconds(seconds) +
                                    std::chrono::milliseconds(milliseconds));
}

}

TEST(ФорматСоответствуетОбразцу) {
    useUtc();

    LogRecord record;
    record.text = "запуск приложения";
    record.level = LogLevel::Warning;
    record.timestamp = makeTime(1700000000, 123);

    const DefaultFormatter formatter;

    CHECK_EQ(formatter.format(record),
             std::string("[2023-11-14 22:13:20.123] [WARNING] "
                         "запуск приложения\n"));
}

TEST(МиллисекундыДополняютсяНулями) {
    useUtc();

    LogRecord record;
    record.text = "x";
    record.level = LogLevel::Info;
    record.timestamp = makeTime(1700000000, 7);

    const DefaultFormatter formatter;

    CHECK_EQ(formatter.format(record),
             std::string("[2023-11-14 22:13:20.007] [INFO] x\n"));
}

TEST(ПустойТекстНеЛомаетФормат) {
    useUtc();

    LogRecord record;
    record.text = "";
    record.level = LogLevel::Error;
    record.timestamp = makeTime(0, 0);

    const DefaultFormatter formatter;

    CHECK_EQ(formatter.format(record),
             std::string("[1970-01-01 00:00:00.000] [ERROR] \n"));
}

TEST(КаждаяЗаписьЗаканчиваетсяПереводомСтроки) {
    LogRecord record;
    record.text = "многострочный\nтекст";
    record.level = LogLevel::Info;
    record.timestamp = std::chrono::system_clock::now();

    const std::string line = DefaultFormatter().format(record);

    CHECK(!line.empty());
    CHECK(line.back() == '\n');
}