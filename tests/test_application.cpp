#include "Fakes.hpp"
#include "Printers.hpp"
#include "TestFramework.hpp"

#include "Application.hpp"

#include <sstream>
#include <string>
#include <vector>

using app::Application;
using app::CliOptions;
using app::LogRequest;
using logger::LogLevel;
using testing::FakeMessageSink;

namespace {

struct Fixture {
    FakeMessageSink sink;
    std::istringstream input;
    std::ostringstream output;

    CliOptions options(LogLevel level = LogLevel::Info) const {
        CliOptions result;
        result.logFile = "test.log";
        result.defaultLevel = level;
        return result;
    }

    std::vector<LogRequest> run(const std::string& lines,
                                LogLevel level = LogLevel::Info) {
        input.str(lines);
        Application application(options(level), sink);
        application.run(input, output);
        return sink.requests();
    }
};

}  // namespace

TEST(КаждаяСтрокаСтановитсяСообщением) {
    Fixture fixture;
    const std::vector<LogRequest> requests = fixture.run("первое\nвторое\n");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(2));
    if (requests.size() == 2) {
        CHECK_EQ(requests[0].text, std::string("первое"));
        CHECK_EQ(requests[1].text, std::string("второе"));
    }
}

TEST(УровеньИзСтрокиПередаётсяДальше) {
    Fixture fixture;
    const std::vector<LogRequest> requests =
        fixture.run("error диск переполнен\n");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(1));
    if (!requests.empty()) {
        CHECK_EQ(requests.front().level, LogLevel::Error);
        CHECK_EQ(requests.front().text, std::string("диск переполнен"));
    }
}

TEST(БезУровняБерётсяТекущий) {
    Fixture fixture;
    const std::vector<LogRequest> requests =
        fixture.run("сообщение\n", LogLevel::Warning);

    CHECK_EQ(requests.size(), static_cast<std::size_t>(1));
    if (!requests.empty()) {
        CHECK_EQ(requests.front().level, LogLevel::Warning);
    }
}

TEST(ПустыеСтрокиПропускаются) {
    Fixture fixture;
    const std::vector<LogRequest> requests = fixture.run("\n   \n\t\nтекст\n");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(1));
}

TEST(КомандаLevelМеняетУровеньПоследующихСообщений) {
    Fixture fixture;
    const std::vector<LogRequest> requests =
        fixture.run("до\n:level error\nпосле\n");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(2));
    if (requests.size() == 2) {
        CHECK_EQ(requests[0].level, LogLevel::Info);
        CHECK_EQ(requests[1].level, LogLevel::Error);
    }
}

TEST(КомандаНеЗаписываетсяКакСообщение) {
    Fixture fixture;
    const std::vector<LogRequest> requests = fixture.run(":level warning\n");

    CHECK(requests.empty());
}

TEST(QuitПрекращаетЧтение) {
    Fixture fixture;
    const std::vector<LogRequest> requests =
        fixture.run("первое\n:quit\nне читается\n");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(1));
}

TEST(КонецВводаЗавершаетЦикл) {
    Fixture fixture;
    const std::vector<LogRequest> requests = fixture.run("только это");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(1));
}

TEST(ВремяФиксируетсяВПорядкеВвода) {
    Fixture fixture;
    const std::vector<LogRequest> requests = fixture.run("a\nb\nc\n");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(3));
    for (std::size_t i = 1; i < requests.size(); ++i) {
        CHECK(requests[i - 1].timestamp <= requests[i].timestamp);
    }
}

TEST(ЭкранированнаяСтрокаЗаписываетсяДословно) {
    Fixture fixture;
    const std::vector<LogRequest> requests = fixture.run("\\error в модуле\n");

    CHECK_EQ(requests.size(), static_cast<std::size_t>(1));
    if (!requests.empty()) {
        CHECK_EQ(requests.front().text, std::string("error в модуле"));
        CHECK_EQ(requests.front().level, LogLevel::Info);
    }
}

TEST(ПриветствиеПоказываетПараметрыИКоманды) {
    Fixture fixture;
    fixture.run("", LogLevel::Warning);

    const std::string text = fixture.output.str();
    CHECK(text.find("test.log") != std::string::npos);
    CHECK(text.find("WARNING") != std::string::npos);
    CHECK(text.find(":quit") != std::string::npos);
}

TEST(ОтказПриёмникаОстанавливаетЦикл) {
    Fixture fixture;
    fixture.sink.close();
    const std::vector<LogRequest> requests = fixture.run("первое\nвторое\n");

    CHECK(requests.empty());
    CHECK(fixture.output.str().find("остановлен") != std::string::npos);
}
