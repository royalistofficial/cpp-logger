#include "Printers.hpp"
#include "TestFramework.hpp"

#include "logger/Logger.hpp"

#include <cstdio>
#include <fstream>
#include <memory>
#include <mutex>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using logger::ILogFormatter;
using logger::ILogSink;
using logger::LogLevel;
using logger::LogRecord;
using logger::Logger;

namespace {

class MemorySink final : public ILogSink {
public:
    void write(std::string_view record) override {
        const std::lock_guard<std::mutex> lock(mutex_);
        lines_.emplace_back(record);
    }

    std::vector<std::string> lines() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return lines_;
    }

private:
    mutable std::mutex mutex_;
    std::vector<std::string> lines_;
};

class PlainFormatter final : public ILogFormatter {
public:
    std::string format(const LogRecord& record) const override {
        return std::string(logger::toString(record.level)) + " " + record.text +
               "\n";
    }
};

struct TestLogger {
    MemorySink* sink;
    std::unique_ptr<Logger> logger;
};

TestLogger makeTestLogger(LogLevel level) {
    auto sink = std::make_unique<MemorySink>();
    MemorySink* raw = sink.get();
    auto logger = std::make_unique<Logger>(
        std::move(sink), std::make_unique<PlainFormatter>(), level);
    return {raw, std::move(logger)};
}

}  

TEST(СообщенияНижеПорогаОтбрасываются) {
    TestLogger test = makeTestLogger(LogLevel::Warning);

    test.logger->log("не пишется", LogLevel::Info);
    test.logger->log("пишется", LogLevel::Warning);
    test.logger->log("тоже пишется", LogLevel::Error);

    const std::vector<std::string> lines = test.sink->lines();

    CHECK_EQ(lines.size(), std::size_t(2));
    CHECK_EQ(lines.at(0), std::string("WARNING пишется\n"));
    CHECK_EQ(lines.at(1), std::string("ERROR тоже пишется\n"));
}

TEST(БезУровняИспользуетсяУровеньПоУмолчанию) {
    TestLogger test = makeTestLogger(LogLevel::Error);

    test.logger->log("без уровня");

    const std::vector<std::string> lines = test.sink->lines();

    CHECK_EQ(lines.size(), std::size_t(1));
    CHECK_EQ(lines.at(0), std::string("ERROR без уровня\n"));
}

TEST(УровеньМеняетсяПослеИнициализации) {
    TestLogger test = makeTestLogger(LogLevel::Error);

    test.logger->log("отброшено", LogLevel::Info);

    test.logger->setDefaultLevel(LogLevel::Info);
    CHECK_EQ(test.logger->defaultLevel(), LogLevel::Info);

    test.logger->log("записано", LogLevel::Info);

    const std::vector<std::string> lines = test.sink->lines();

    CHECK_EQ(lines.size(), std::size_t(1));
    CHECK_EQ(lines.at(0), std::string("INFO записано\n"));
}

TEST(ПустыеЗависимостиОтвергаются) {
    bool sinkRejected = false;
    try {
        Logger bad(nullptr, std::make_unique<PlainFormatter>(), LogLevel::Info);
    } catch (const std::invalid_argument&) {
        sinkRejected = true;
    }

    bool formatterRejected = false;
    try {
        Logger bad(std::make_unique<MemorySink>(), nullptr, LogLevel::Info);
    } catch (const std::invalid_argument&) {
        formatterRejected = true;
    }

    CHECK(sinkRejected);
    CHECK(formatterRejected);
}

TEST(ЗаписьИзЧетырёхПотоковНеПеремешивается) {
    const std::string path = "/tmp/logger_concurrent.log";
    std::remove(path.c_str());

    constexpr int kThreads = 4;
    constexpr int kLinesPerThread = 1000;

    {
        std::unique_ptr<logger::ILogger> log =
            logger::makeFileLogger(path, LogLevel::Info);

        std::vector<std::thread> workers;
        workers.reserve(kThreads);

        for (int id = 0; id < kThreads; ++id) {
            workers.emplace_back([&log, id] {
                for (int i = 0; i < kLinesPerThread; ++i) {
                    log->log("поток " + std::to_string(id) + " сообщение " +
                                 std::to_string(i),
                             LogLevel::Info);
                }
            });
        }

        for (std::thread& worker : workers) {
            worker.join();
        }
    }

    const std::regex pattern(
        R"(^\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] \[INFO\] )"
        R"(поток (\d) сообщение (\d+)$)");

    std::ifstream in(path);
    std::string line;
    int total = 0;
    int malformed = 0;
    std::vector<int> perThread(kThreads, 0);

    while (std::getline(in, line)) {
        ++total;

        std::smatch match;
        if (!std::regex_match(line, match, pattern)) {
            ++malformed;
            continue;
        }

        const int id = std::stoi(match[1].str());
        if (id >= 0 && id < kThreads) {
            ++perThread[static_cast<std::size_t>(id)];
        }
    }
    in.close();
    std::remove(path.c_str());

    CHECK_EQ(total, kThreads * kLinesPerThread);
    CHECK_EQ(malformed, 0);

    for (int id = 0; id < kThreads; ++id) {
        CHECK_EQ(perThread[static_cast<std::size_t>(id)], kLinesPerThread);
    }
}

TEST(СменаУровняВоВремяЗаписиБезопасна) {
    TestLogger test = makeTestLogger(LogLevel::Info);

    std::thread writer([&test] {
        for (int i = 0; i < 2000; ++i) {
            test.logger->log("сообщение", LogLevel::Error);
        }
    });

    std::thread switcher([&test] {
        for (int i = 0; i < 2000; ++i) {
            test.logger->setDefaultLevel(i % 2 == 0 ? LogLevel::Info
                                                    : LogLevel::Error);
        }
    });

    writer.join();
    switcher.join();

    CHECK_EQ(test.sink->lines().size(), std::size_t(2000));
}
