#pragma once

#include "IMessageSink.hpp"

#include "logger/ILogSink.hpp"
#include "logger/ILogger.hpp"

#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace testing {

class MemorySink final : public logger::ILogSink {
public:
    void write(std::string_view record) override {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (failFrom_ != 0 && lines_.size() + 1 >= failFrom_) {
            throw std::runtime_error("имитация сбоя записи");
        }
        lines_.emplace_back(record);
    }

    void failFrom(std::size_t index) {
        const std::lock_guard<std::mutex> lock(mutex_);
        failFrom_ = index;
    }

    std::vector<std::string> lines() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return lines_;
    }

    std::size_t count() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return lines_.size();
    }

private:
    mutable std::mutex mutex_;
    std::vector<std::string> lines_;
    std::size_t failFrom_ = 0;
};

class FakeLogger final : public logger::ILogger {
public:
    struct Entry {
        std::string text;
        logger::LogLevel level;
        std::chrono::system_clock::time_point timestamp;
    };

    void log(std::string_view message, logger::LogLevel level) override {
        log(message, level, std::chrono::system_clock::now());
    }

    void log(std::string_view message) override {
        log(message, defaultLevel());
    }

    void log(std::string_view message, logger::LogLevel level,
             std::chrono::system_clock::time_point timestamp) override {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (failAll_) {
            throw std::runtime_error("имитация сбоя журнала");
        }
        entries_.push_back({std::string(message), level, timestamp});
    }

    void setDefaultLevel(logger::LogLevel level) noexcept override {
        const std::lock_guard<std::mutex> lock(mutex_);
        levelChanges_.push_back(level);
        defaultLevel_ = level;
    }

    logger::LogLevel defaultLevel() const noexcept override {
        const std::lock_guard<std::mutex> lock(mutex_);
        return defaultLevel_;
    }

    void failAll(bool value) {
        const std::lock_guard<std::mutex> lock(mutex_);
        failAll_ = value;
    }

    std::vector<Entry> entries() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return entries_;
    }

    std::vector<logger::LogLevel> levelChanges() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return levelChanges_;
    }

private:
    mutable std::mutex mutex_;
    std::vector<Entry> entries_;
    std::vector<logger::LogLevel> levelChanges_;
    logger::LogLevel defaultLevel_ = logger::LogLevel::Info;
    bool failAll_ = false;
};

class FakeMessageSink final : public app::IMessageSink {
public:
    bool submit(app::LogRequest request) override {
        if (closed_) {
            ++stats_.dropped;
            return false;
        }
        requests_.push_back(std::move(request));
        ++stats_.processed;
        return true;
    }

    bool setDefaultLevel(logger::LogLevel level) override {
        if (closed_) {
            return false;
        }
        levels_.push_back(level);
        return true;
    }

    app::WriterStats stats() const override { return stats_; }

    void close() { closed_ = true; }

    const std::vector<app::LogRequest>& requests() const { return requests_; }
    const std::vector<logger::LogLevel>& levels() const { return levels_; }

private:
    std::vector<app::LogRequest> requests_;
    std::vector<logger::LogLevel> levels_;
    app::WriterStats stats_;
    bool closed_ = false;
};

}  // namespace testing
