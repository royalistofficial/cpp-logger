#include "Printers.hpp"
#include "TestFramework.hpp"

#include "logger/LogLevel.hpp"

#include <optional>
#include <string>

using logger::LogLevel;
using logger::parseLevel;
using logger::toString;

TEST(ToStringДаётКаноническиеИмена) {
    CHECK_EQ(toString(LogLevel::Info), std::string_view("INFO"));
    CHECK_EQ(toString(LogLevel::Warning), std::string_view("WARNING"));
    CHECK_EQ(toString(LogLevel::Error), std::string_view("ERROR"));
}

TEST(УровниУпорядоченыПоВажности) {
    CHECK(LogLevel::Info < LogLevel::Warning);
    CHECK(LogLevel::Warning < LogLevel::Error);
    CHECK(LogLevel::Error >= LogLevel::Info);
}

TEST(РазборНеЗависитОтРегистра) {
    CHECK_EQ(parseLevel("info"), std::optional<LogLevel>(LogLevel::Info));
    CHECK_EQ(parseLevel("INFO"), std::optional<LogLevel>(LogLevel::Info));
    CHECK_EQ(parseLevel("InFo"), std::optional<LogLevel>(LogLevel::Info));
    CHECK_EQ(parseLevel("ERROR"), std::optional<LogLevel>(LogLevel::Error));
    CHECK_EQ(parseLevel("error"), std::optional<LogLevel>(LogLevel::Error));
}

TEST(РазборПринимаетКраткиеФормы) {
    CHECK_EQ(parseLevel("warn"), std::optional<LogLevel>(LogLevel::Warning));
    CHECK_EQ(parseLevel("WARNING"), std::optional<LogLevel>(LogLevel::Warning));
    CHECK_EQ(parseLevel("err"), std::optional<LogLevel>(LogLevel::Error));
}

TEST(РазборОтвергаетМусор) {
    CHECK_EQ(parseLevel(""), std::optional<LogLevel>());
    CHECK_EQ(parseLevel("   "), std::optional<LogLevel>());
    CHECK_EQ(parseLevel("debug"), std::optional<LogLevel>());
    CHECK_EQ(parseLevel("info!"), std::optional<LogLevel>());
    CHECK_EQ(parseLevel("inf"), std::optional<LogLevel>());
    CHECK_EQ(parseLevel("42"), std::optional<LogLevel>());
    CHECK_EQ(parseLevel("информация"), std::optional<LogLevel>());
}

TEST(РазборИОбратноеПреобразованиеСогласованы) {
    for (LogLevel level : {LogLevel::Info, LogLevel::Warning, LogLevel::Error}) {
        CHECK_EQ(parseLevel(toString(level)), std::optional<LogLevel>(level));
    }
}
