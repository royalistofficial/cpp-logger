#include "Fakes.hpp"
#include "Printers.hpp"
#include "TestFramework.hpp"

#include "logger/DefaultFormatter.hpp"
#include "logger/Logger.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using logger::LogLevel;
using logger::Logger;
using testing::MemorySink;

namespace {

struct Fixture {
    MemorySink* sink = nullptr;
    std::unique_ptr<Logger> log;
};

Fixture makeLogger(LogLevel level) {
    auto sink = std::make_unique<MemorySink>();
    MemorySink* raw = sink.get();

    Fixture fixture;
    fixture.sink = raw;
    fixture.log = std::make_unique<Logger>(
        std::move(sink), std::make_unique<logger::DefaultFormatter>(), level);
    return fixture;
}

}  // namespace

TEST(СообщенияНижеПорогаОтбрасываются) {
    Fixture fixture = makeLogger(LogLevel::Warning);

    fixture.log->log("не пройдёт", LogLevel::Info);
    fixture.log->log("пройдёт", LogLevel::Warning);
    fixture.log->log("тоже пройдёт", LogLevel::Error);

    CHECK_EQ(fixture.sink->count(), static_cast<std::size_t>(2));
}

TEST(БезУровняИспользуетсяУровеньПоУмолчанию) {
    Fixture fixture = makeLogger(LogLevel::Error);

    fixture.log->log("без уровня");

    const std::vector<std::string> lines = fixture.sink->lines();
    CHECK_EQ(lines.size(), static_cast<std::size_t>(1));
    if (!lines.empty()) {
        CHECK(lines.front().find("[ERROR]") != std::string::npos);
    }
}

TEST(СообщениеБезУровняНеОтбрасываетсяСобственнымПорогом) {
    // Порог читается один раз и сразу становится уровнем сообщения:
    // повторная проверка привела бы к потере записи.
    Fixture fixture = makeLogger(LogLevel::Error);

    for (int i = 0; i < 100; ++i) {
        fixture.log->log("должно записаться");
    }

    CHECK_EQ(fixture.sink->count(), static_cast<std::size_t>(100));
}

TEST(УровеньМеняетсяПослеИнициализации) {
    Fixture fixture = makeLogger(LogLevel::Error);

    fixture.log->log("отброшено", LogLevel::Info);
    fixture.log->setDefaultLevel(LogLevel::Info);
    CHECK_EQ(fixture.log->defaultLevel(), LogLevel::Info);
    fixture.log->log("записано", LogLevel::Info);

    CHECK_EQ(fixture.sink->count(), static_cast<std::size_t>(1));
}

TEST(ЗаданноеВремяПопадаетВЗапись) {
    Fixture fixture = makeLogger(LogLevel::Info);

    const auto moment = std::chrono::system_clock::time_point(
        std::chrono::seconds(0) + std::chrono::milliseconds(123));
    fixture.log->log("текст", LogLevel::Info, moment);

    const std::vector<std::string> lines = fixture.sink->lines();
    CHECK_EQ(lines.size(), static_cast<std::size_t>(1));
    if (!lines.empty()) {
        CHECK(lines.front().find(".123]") != std::string::npos);
    }
}

TEST(ПустыеЗависимостиОтвергаются) {
    CHECK_THROWS_AS(Logger(nullptr,
                           std::make_unique<logger::DefaultFormatter>(),
                           LogLevel::Info),
                    std::invalid_argument);
    CHECK_THROWS_AS(Logger(std::make_unique<MemorySink>(), nullptr,
                           LogLevel::Info),
                    std::invalid_argument);
}

TEST(ОшибкаПриёмникаПробрасываетсяНаружу) {
    Fixture fixture = makeLogger(LogLevel::Info);
    fixture.sink->failFrom(1);

    CHECK_THROWS_AS(fixture.log->log("сбой", LogLevel::Info),
                    std::runtime_error);
}

TEST(ЗаписьИзЧетырёхПотоковНеПеремешивается) {
    Fixture fixture = makeLogger(LogLevel::Info);

    constexpr int kThreads = 4;
    constexpr int kPerThread = 250;

    std::vector<std::thread> workers;
    workers.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        workers.emplace_back([&fixture, t] {
            for (int i = 0; i < kPerThread; ++i) {
                fixture.log->log("поток-" + std::to_string(t), LogLevel::Info);
            }
        });
    }
    for (std::thread& worker : workers) {
        worker.join();
    }

    const std::vector<std::string> lines = fixture.sink->lines();
    CHECK_EQ(lines.size(), static_cast<std::size_t>(kThreads * kPerThread));

    // Каждая строка целая: ровно один перевод строки и корректный хвост.
    for (const std::string& line : lines) {
        CHECK_EQ(std::count(line.begin(), line.end(), '\n'),
                 static_cast<std::ptrdiff_t>(1));
        CHECK(line.find("[INFO] поток-") != std::string::npos);
    }
}

TEST(СменаУровняВоВремяЗаписиБезопасна) {
    Fixture fixture = makeLogger(LogLevel::Info);
    std::atomic<bool> stop{false};

    std::thread switcher([&fixture, &stop] {
        while (!stop.load()) {
            fixture.log->setDefaultLevel(LogLevel::Error);
            fixture.log->setDefaultLevel(LogLevel::Info);
        }
    });

    for (int i = 0; i < 500; ++i) {
        fixture.log->log("сообщение", LogLevel::Error);
    }

    stop.store(true);
    switcher.join();

    // Уровень Error проходит любой порог, поэтому потерь быть не должно.
    CHECK_EQ(fixture.sink->count(), static_cast<std::size_t>(500));
}
