#include "AsyncLogWriter.hpp"

#include <exception>
#include <optional>
#include <ostream>
#include <utility>

namespace app {

AsyncLogWriter::AsyncLogWriter(logger::ILogger& log, std::ostream& errors,
                               std::size_t capacity)
    : log_(log), errors_(errors), queue_(capacity), worker_([this] { run(); }) {}

AsyncLogWriter::~AsyncLogWriter() {
    finish();
}

bool AsyncLogWriter::submit(LogRequest request) {
    if (queue_.push(QueueItem{std::move(request)})) {
        return true;
    }
    dropped_.fetch_add(1, std::memory_order_relaxed);
    return false;
}

bool AsyncLogWriter::setDefaultLevel(logger::LogLevel level) {
    return queue_.push(QueueItem{SetLevelCommand{level}});
}

WriterStats AsyncLogWriter::stats() const {
    WriterStats result;
    result.processed = processed_.load(std::memory_order_relaxed);
    result.failed = failed_.load(std::memory_order_relaxed);
    result.dropped = dropped_.load(std::memory_order_relaxed);
    return result;
}

WriterStats AsyncLogWriter::finish() {
    queue_.close();
    if (worker_.joinable()) {
        worker_.join();
    }
    return stats();
}

void AsyncLogWriter::run() {
    while (std::optional<QueueItem> item = queue_.pop()) {
        std::visit([this](const auto& value) { handle(value); }, *item);
    }
}

void AsyncLogWriter::handle(const LogRequest& request) {
    try {
        log_.log(request.text, request.level, request.timestamp);
        processed_.fetch_add(1, std::memory_order_relaxed);
    } catch (const std::exception& error) {
        failed_.fetch_add(1, std::memory_order_relaxed);

        errors_ << "ошибка записи в журнал: " << error.what()
                << " (сообщение потеряно: " << request.text << ")\n";
    }
}

void AsyncLogWriter::handle(const SetLevelCommand& command) {
    log_.setDefaultLevel(command.level);
}

}  // namespace app
