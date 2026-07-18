#include "Fakes.hpp"
#include "Printers.hpp"
#include "TestFramework.hpp"

#include "AsyncLogWriter.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using app::AsyncLogWriter;
using app::LogRequest;
using app::WriterStats;
using logger::LogLevel;
using testing::FakeLogger;

namespace {

LogRequest makeRequest(std::string text, LogLevel level = LogLevel::Info) {
    LogRequest request;
    request.text = std::move(text);
    request.level = level;
    request.timestamp = std::chrono::system_clock::now();
    return request;
}

}  // namespace

TEST(СообщенияДоходятДоЖурнала) {
    FakeLogger log;
    std::ostringstream errors;
    WriterStats stats;
    {
        AsyncLogWriter writer(log, errors);
        CHECK(writer.submit(makeRequest("первое")));
        CHECK(writer.submit(makeRequest("второе", LogLevel::Error)));
        stats = writer.finish();
    }

    const std::vector<FakeLogger::Entry> entries = log.entries();
    CHECK_EQ(entries.size(), static_cast<std::size_t>(2));
    CHECK_EQ(stats.processed, static_cast<std::size_t>(2));
    CHECK_EQ(stats.failed, static_cast<std::size_t>(0));
    if (entries.size() == 2) {
        CHECK_EQ(entries[0].text, std::string("первое"));
        CHECK_EQ(entries[1].level, LogLevel::Error);
    }
    CHECK(errors.str().empty());
}

TEST(ВремяПолученияСохраняетсяАНеВремяЗаписи) {
    FakeLogger log;
    std::ostringstream errors;

    const auto moment =
        std::chrono::system_clock::now() - std::chrono::hours(1);
    {
        AsyncLogWriter writer(log, errors);
        LogRequest request = makeRequest("старое");
        request.timestamp = moment;
        writer.submit(std::move(request));
        writer.finish();
    }

    const std::vector<FakeLogger::Entry> entries = log.entries();
    CHECK_EQ(entries.size(), static_cast<std::size_t>(1));
    if (!entries.empty()) {
        CHECK(entries.front().timestamp == moment);
    }
}

TEST(СменаУровняИдётТемЖеКаналом) {
    FakeLogger log;
    std::ostringstream errors;
    {
        AsyncLogWriter writer(log, errors);
        writer.submit(makeRequest("до"));
        writer.setDefaultLevel(LogLevel::Error);
        writer.submit(makeRequest("после"));
        writer.finish();
    }

    const std::vector<LogLevel> changes = log.levelChanges();
    CHECK_EQ(changes.size(), static_cast<std::size_t>(1));
    if (!changes.empty()) {
        CHECK_EQ(changes.front(), LogLevel::Error);
    }
    // Порядок соблюдён: первое сообщение записано до смены уровня.
    CHECK_EQ(log.entries().size(), static_cast<std::size_t>(2));
}

TEST(ОшибкаЗаписиСчитаетсяИНеПрерываетРаботу) {
    FakeLogger log;
    std::ostringstream errors;
    WriterStats stats;
    {
        AsyncLogWriter writer(log, errors);
        log.failAll(true);
        writer.submit(makeRequest("потеряется"));

        // Дожидаемся обработки, затем чиним журнал.
        while (writer.stats().failed == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        log.failAll(false);
        writer.submit(makeRequest("запишется"));
        stats = writer.finish();
    }

    CHECK_EQ(stats.failed, static_cast<std::size_t>(1));
    CHECK_EQ(stats.processed, static_cast<std::size_t>(1));
    CHECK(errors.str().find("потеряется") != std::string::npos);
}

TEST(ПослеЗавершенияСообщенияНеПринимаются) {
    FakeLogger log;
    std::ostringstream errors;

    AsyncLogWriter writer(log, errors);
    writer.finish();

    CHECK(!writer.submit(makeRequest("поздно")));
    CHECK_EQ(writer.stats().dropped, static_cast<std::size_t>(1));
    CHECK_EQ(log.entries().size(), static_cast<std::size_t>(0));
}

TEST(ПовторноеЗавершениеБезопасно) {
    FakeLogger log;
    std::ostringstream errors;

    AsyncLogWriter writer(log, errors);
    writer.submit(makeRequest("одно"));

    const WriterStats first = writer.finish();
    const WriterStats second = writer.finish();

    CHECK_EQ(first.processed, static_cast<std::size_t>(1));
    CHECK_EQ(second.processed, first.processed);
}

TEST(ВсёПринятоеДописываетсяПередОстановкой) {
    FakeLogger log;
    std::ostringstream errors;
    constexpr int kCount = 2000;
    WriterStats stats;
    {
        AsyncLogWriter writer(log, errors, 32);
        for (int i = 0; i < kCount; ++i) {
            writer.submit(makeRequest("сообщение-" + std::to_string(i)));
        }
        stats = writer.finish();
    }

    CHECK_EQ(stats.processed, static_cast<std::size_t>(kCount));
    CHECK_EQ(log.entries().size(), static_cast<std::size_t>(kCount));
}

TEST(ПорядокСообщенийСохраняется) {
    FakeLogger log;
    std::ostringstream errors;
    {
        AsyncLogWriter writer(log, errors, 4);
        for (int i = 0; i < 200; ++i) {
            writer.submit(makeRequest(std::to_string(i)));
        }
        writer.finish();
    }

    const std::vector<FakeLogger::Entry> entries = log.entries();
    CHECK_EQ(entries.size(), static_cast<std::size_t>(200));
    for (std::size_t i = 0; i < entries.size(); ++i) {
        CHECK_EQ(entries[i].text, std::to_string(i));
    }
}
