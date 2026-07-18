#include "Fakes.hpp"
#include "Printers.hpp"
#include "TestFramework.hpp"

#include "CommandProcessor.hpp"

#include <sstream>
#include <string>

using app::CommandProcessor;
using logger::LogLevel;
using testing::FakeMessageSink;

namespace {

struct Fixture {
    FakeMessageSink sink;
    LogLevel level = LogLevel::Info;
    std::ostringstream output;
    CommandProcessor processor{sink, level};

    CommandProcessor::Result run(const std::string& line) {
        return processor.execute(line, output);
    }

    std::string text() const { return output.str(); }
};

}  // namespace

TEST(ОбычнаяСтрокаНеКоманда) {
    Fixture fixture;
    CHECK(fixture.run("просто сообщение") ==
          CommandProcessor::Result::NotACommand);
    CHECK(fixture.run("") == CommandProcessor::Result::NotACommand);
    CHECK(fixture.text().empty());
}

TEST(QuitЗавершаетРаботу) {
    Fixture fixture;
    CHECK(fixture.run(":quit") == CommandProcessor::Result::Quit);
}

TEST(ПробелыВокругКомандыИгнорируются) {
    Fixture fixture;
    CHECK(fixture.run("   :quit  ") == CommandProcessor::Result::Quit);
}

TEST(LevelМеняетУровеньИСообщаетПриёмнику) {
    Fixture fixture;
    CHECK(fixture.run(":level warning") == CommandProcessor::Result::Handled);

    CHECK_EQ(fixture.level, LogLevel::Warning);
    CHECK_EQ(fixture.sink.levels().size(), static_cast<std::size_t>(1));
    if (!fixture.sink.levels().empty()) {
        CHECK_EQ(fixture.sink.levels().front(), LogLevel::Warning);
    }
    CHECK(fixture.text().find("WARNING") != std::string::npos);
}

TEST(LevelБезАргументаПодсказывает) {
    Fixture fixture;
    CHECK(fixture.run(":level") == CommandProcessor::Result::Handled);

    CHECK_EQ(fixture.level, LogLevel::Info);
    CHECK(fixture.sink.levels().empty());
    CHECK(fixture.text().find(":level") != std::string::npos);
}

TEST(LevelСМусоромНеМеняетУровень) {
    Fixture fixture;
    CHECK(fixture.run(":level trace") == CommandProcessor::Result::Handled);

    CHECK_EQ(fixture.level, LogLevel::Info);
    CHECK(fixture.sink.levels().empty());
    CHECK(fixture.text().find("trace") != std::string::npos);
}

TEST(НеизвестнаяКомандаНеПопадаетВЖурнал) {
    // Опечатка вроде :qiut не должна незаметно записаться как сообщение.
    Fixture fixture;
    CHECK(fixture.run(":qiut") == CommandProcessor::Result::Handled);
    CHECK(fixture.text().find("неизвестная команда") != std::string::npos);
}

TEST(СправкаПеречисляетВсеКоманды) {
    const std::string help = CommandProcessor::helpText();

    CHECK(help.find(":quit") != std::string::npos);
    CHECK(help.find(":level") != std::string::npos);
    CHECK(help.find(":help") != std::string::npos);
}

TEST(HelpПечатаетСправку) {
    Fixture fixture;
    CHECK(fixture.run(":help") == CommandProcessor::Result::Handled);
    CHECK_EQ(fixture.text(), CommandProcessor::helpText());
}
