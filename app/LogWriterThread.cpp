#include "LogWriterThread.hpp"

#include <exception>
#include <iostream>
#include <optional>

namespace app {

LogWriterThread::LogWriterThread(LogQueue& queue, logger::ILogger& log)
    : queue_(queue), log_(log), worker_([this] { run(); }) {}

LogWriterThread::~LogWriterThread() {
    join();
}

void LogWriterThread::join() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

void LogWriterThread::run() {
    while (std::optional<QueueItem> item = queue_.pop()) {
        std::visit([this](const auto& value) { handle(value); }, *item);
    }
}

void LogWriterThread::handle(const LogRequest& request) {
    try {
        log_.log(request.text, request.level, request.timestamp);
        processed_.fetch_add(1, std::memory_order_relaxed);
    } catch (const std::exception& error) {
        failed_.fetch_add(1, std::memory_order_relaxed);

        std::cerr << "ошибка записи в журнал: " << error.what()
                  << " (сообщение потеряно: " << request.text << ")\n";
    }
}

void LogWriterThread::handle(const SetLevelCommand& command) {
    log_.setDefaultLevel(command.level);
}

std::size_t LogWriterThread::processed() const noexcept {
    return processed_.load(std::memory_order_relaxed);
}

std::size_t LogWriterThread::failed() const noexcept {
    return failed_.load(std::memory_order_relaxed);
}

} 