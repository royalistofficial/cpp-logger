#include "logger/FileLogSink.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>

namespace logger {
namespace {

/// Собирает сообщение об ошибке, дополняя его пояснением от системы.
/// errno читается сразу, поэтому вызывать функцию нужно вплотную к сбою.
std::string describeError(const std::string& action, const std::string& path) {
    std::string message = action + " \"" + path + "\"";

    if (errno != 0) {
        message += ": ";
        message += std::strerror(errno);
    }
    return message;
}

}  // namespace

FileLogSink::FileLogSink(std::string path) : path_(std::move(path)) {
    if (path_.empty()) {
        throw std::runtime_error("не задано имя файла журнала");
    }

    errno = 0;
    stream_.open(path_, std::ios::out | std::ios::app);

    if (!stream_.is_open()) {
        throw std::runtime_error(
            describeError("не удалось открыть файл журнала", path_));
    }
}

void FileLogSink::write(std::string_view record) {
    errno = 0;

    stream_.write(record.data(), static_cast<std::streamsize>(record.size()));
    stream_.flush();

    if (!stream_) {
        // Состояние ошибки сбрасывается, чтобы последующие вызовы могли
        // отработать, если причина сбоя была временной.
        stream_.clear();
        throw std::runtime_error(
            describeError("ошибка записи в файл журнала", path_));
    }
}

}  // namespace logger
