#include "Printers.hpp"
#include "TestFramework.hpp"

#include "Application.hpp"
#include "LogWriterThread.hpp"

#include "logger/ILogger.hpp"

#include <cstdio>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <string>
#include <vector>

using app::Application;
using app::CliOptions;
using app::LogQueue;
using app::LogRequest;
using app::LogWriterThread;
using app::SetLevelCommand;
using logger::LogLevel;

namespace {

class FakeLogger final : public logger::ILogger {
public:
    struct Entry {
        std::string text;
        LogLevel level;
    };

    void log(std::string_view message, LogLevel level) override {
        log(message, level, std::chrono::system_clock::now());
    }

    void log(std::string_view message) override {
        log(message, defaultLevel());
    }

    void log(std::string_view message, LogLevel level,
             std::chrono::system_clock::time_point) override {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (failing_) {
            throw std::runtime_error("диск недоступен");
        }
        if (level >= level_) {
            entries_.push_back({std::string(message), level});
        }
    }

    void setDefaultLevel(LogLevel level) noexcept override {
        const std::lock_guard<std::mutex> lock(mutex_);
        level_ = level;
    }

    LogLevel defaultLevel() const noexcept override {
        const std::lock_guard<std::mutex> lock(mutex_);
        return level_;
    }

    void setFailing(bool failing) {
        const std::lock_guard<std::mutex> lock(mutex_);
        failing_ = failing;
    }

    std::vector<Entry> entries() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return entries_;
    }

private:
    mutable std::mutex mutex_;
    std::vector<Entry> entries_;
    LogLevel level_ = LogLevel::Info;
    bool failing_ = false;
};

LogRequest makeRequest(std::string text, LogLevel level) {
    LogRequest request;
    request.text = std::move(text);
    request.level = level;
    request.timestamp = std::chrono::system_clock::now();
    return request;
}

class TempLog {
public:
    explicit TempLog(std::string name) : path_("/tmp/" + name) {
        std::remove(path_.c_str());
    }
    ~TempLog() { std::remove(path_.c_str()); }

    TempLog(const TempLog&) = delete;
    TempLog& operator=(const TempLog&) = delete;

    const std::string& path() const noexcept { return path_; }

    std::vector<std::string> lines() const {
        std::ifstream in(path_);
        std::vector<std::string> result;
        std::string line;
        while (std::getline(in, line)) {
            result.push_back(line);
        }
        return result;
    }

private:
    std::string path_;
};

}  // namespace

TEST(ПотокЗаписиОбрабатываетВсюОчередь) {
    FakeLogger fake;
    LogQueue queue;

    {
        LogWriterThread writer(queue, fake);

        queue.push(makeRequest("первое", LogLevel::Info));
        queue.push(makeRequest("второе", LogLevel::Error));
        queue.close();
    }  // Деструктор дожидается завершения потока.

    const std::vector<FakeLogger::Entry> entries = fake.entries();

    CHECK_EQ(entries.size(), std::size_t(2));
    if (entries.size() == 2) {
        CHECK_EQ(entries[0].text, std::string("первое"));
        CHECK_EQ(entries[1].text, std::string("второе"));
    }
}

TEST(ОшибкаЗаписиНеОстанавливаетПоток) {
    FakeLogger fake;
    LogQueue queue;

    fake.setFailing(true);

    std::ostringstream captured;
    std::streambuf* saved = std::cerr.rdbuf(captured.rdbuf());

    LogWriterThread writer(queue, fake);
    queue.push(makeRequest("потеряется", LogLevel::Error));

    while (writer.failed() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    fake.setFailing(false);

    queue.push(makeRequest("запишется", LogLevel::Error));
    queue.close();
    writer.join();

    std::cerr.rdbuf(saved);

    CHECK_EQ(writer.failed(), std::size_t(1));
    CHECK_EQ(writer.processed(), std::size_t(1));
    CHECK_EQ(fake.entries().size(), std::size_t(1));
    CHECK(captured.str().find("потеряется") != std::string::npos);
}

TEST(КомандаСменыУровняПрименяетсяВПорядкеОчереди) {
    FakeLogger fake;
    LogQueue queue;

    {
        LogWriterThread writer(queue, fake);

        queue.push(makeRequest("до смены", LogLevel::Info));
        queue.push(SetLevelCommand{LogLevel::Error});
        queue.push(makeRequest("после смены", LogLevel::Info));
        queue.push(makeRequest("важное", LogLevel::Error));
        queue.close();
    }

    const std::vector<FakeLogger::Entry> entries = fake.entries();

    CHECK_EQ(entries.size(), std::size_t(2));
    if (entries.size() == 2) {
        CHECK_EQ(entries[0].text, std::string("до смены"));
        CHECK_EQ(entries[1].text, std::string("важное"));
    }
    CHECK_EQ(fake.defaultLevel(), LogLevel::Error);
}

TEST(ПриложениеЗаписываетВведённыеСообщения) {
    TempLog log("logger_app_basic.log");

    CliOptions options;
    options.logFile = log.path();
    options.defaultLevel = LogLevel::Info;

    std::istringstream input(
        "первое сообщение\n"
        "ERROR диск переполнен\n"
        "\n" 
        "\\error дословно\n" 
    );
    std::ostringstream output;

    const int code = Application(options).run(input, output);
    CHECK_EQ(code, 0);

    const std::vector<std::string> lines = log.lines();
    CHECK_EQ(lines.size(), std::size_t(3));

    if (lines.size() == 3) {
        CHECK(lines[0].find("[INFO] первое сообщение") != std::string::npos);
        CHECK(lines[1].find("[ERROR] диск переполнен") != std::string::npos);
        CHECK(lines[2].find("[INFO] error дословно") != std::string::npos);
    }
}

TEST(ПриложениеОтбрасываетСообщенияНижеПорога) {
    TempLog log("logger_app_filter.log");

    CliOptions options;
    options.logFile = log.path();
    options.defaultLevel = LogLevel::Warning;

    std::istringstream input(
        "INFO это не попадёт\n"
        "WARNING это попадёт\n");
    std::ostringstream output;

    Application(options).run(input, output);

    const std::vector<std::string> lines = log.lines();

    CHECK_EQ(lines.size(), std::size_t(1));
    if (!lines.empty()) {
        CHECK(lines[0].find("это попадёт") != std::string::npos);
    }
}

TEST(КомандаLevelМеняетПорогВоВремяРаботы) {
    TempLog log("logger_app_level.log");

    CliOptions options;
    options.logFile = log.path();
    options.defaultLevel = LogLevel::Info;

    std::istringstream input(
        "INFO до смены\n"
        ":level error\n"
        "INFO после смены\n"
        "ERROR важное\n");
    std::ostringstream output;

    Application(options).run(input, output);

    const std::vector<std::string> lines = log.lines();

    CHECK_EQ(lines.size(), std::size_t(2));
    if (lines.size() == 2) {
        CHECK(lines[0].find("до смены") != std::string::npos);
        CHECK(lines[1].find("важное") != std::string::npos);
    }
}

TEST(КомандаQuitЗавершаетРаботу) {
    TempLog log("logger_app_quit.log");

    CliOptions options;
    options.logFile = log.path();
    options.defaultLevel = LogLevel::Info;

    std::istringstream input(
        "до выхода\n"
        ":quit\n"
        "после выхода\n");  // не должно быть прочитано
    std::ostringstream output;

    const int code = Application(options).run(input, output);

    CHECK_EQ(code, 0);
    CHECK_EQ(log.lines().size(), std::size_t(1));
}

TEST(НедоступныйЖурналДаётКодОшибки) {
    CliOptions options;
    options.logFile = "/nonexistent_directory_12345/app.log";
    options.defaultLevel = LogLevel::Info;

    std::istringstream input("сообщение\n");
    std::ostringstream output;

    std::ostringstream captured;
    std::streambuf* saved = std::cerr.rdbuf(captured.rdbuf());

    const int code = Application(options).run(input, output);

    std::cerr.rdbuf(saved);

    CHECK_EQ(code, 1);
    CHECK(!captured.str().empty());
}
