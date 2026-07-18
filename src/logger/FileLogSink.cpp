#include "logger/FileLogSink.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace logger {
namespace {

/**
 * @brief Собирает сообщение об ошибке, дополняя его пояснением системы.
 */
std::string describeError(std::string_view action, const std::string& path,
                          int errorCode) {
    std::string message;
    message.reserve(action.size() + path.size() + 64);

    message += action;
    message += " \"";
    message += path;
    message += '"';

    if (errorCode != 0) {
        message += ": ";
        message += std::strerror(errorCode);
    }
    return message;
}

}  // namespace

FileLogSink::FileLogSink(std::string path, FlushPolicy flushPolicy)
    : path_(std::move(path)), flushPolicy_(flushPolicy) {
    if (path_.empty()) {
        throw std::runtime_error("не задано имя файла журнала");
    }

    errno = 0;
    stream_.open(path_, std::ios::out | std::ios::app);
    const int errorCode = errno;

    if (!stream_.is_open()) {
        throw std::runtime_error(
            describeError("не удалось открыть файл журнала", path_, errorCode));
    }
}

void FileLogSink::write(std::string_view record) {
    errno = 0;

    stream_.write(record.data(), static_cast<std::streamsize>(record.size()));
    if (flushPolicy_ == FlushPolicy::EveryRecord) {
        stream_.flush();
    }
    const int errorCode = errno;

    if (!stream_) {
        stream_.clear();
        throw std::runtime_error(
            describeError("ошибка записи в файл журнала", path_, errorCode));
    }
}

}  // namespace logger
