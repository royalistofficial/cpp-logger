#pragma once

#include "BlockingQueue.hpp"
#include "IMessageSink.hpp"

#include "logger/ILogger.hpp"

#include <atomic>
#include <iosfwd>
#include <thread>
#include <variant>

namespace app {

/**
 * @brief Асинхронный приёмник: очередь плюс отдельный поток записи.
 */
class AsyncLogWriter final : public IMessageSink {
public:
    static constexpr std::size_t kDefaultCapacity = 8192;

    /**
     * @param log Журнал, в который выполняется запись. Должен пережить объект.
     * @param errors Поток для сообщений об ошибках записи.
     * @param capacity Максимальный размер очереди.
     */
    AsyncLogWriter(logger::ILogger& log, std::ostream& errors,
                   std::size_t capacity = kDefaultCapacity);

    ~AsyncLogWriter() override;

    bool submit(LogRequest request) override;
    bool setDefaultLevel(logger::LogLevel level) override;
    WriterStats stats() const override;

    /**
     * @brief Закрывает очередь, дожидается записи принятых сообщений.
     * @return Итоговые счётчики. Повторный вызов безопасен.
     */
    WriterStats finish();

private:
    struct SetLevelCommand {
        logger::LogLevel level = logger::LogLevel::Info;
    };

    using QueueItem = std::variant<LogRequest, SetLevelCommand>;

    void run();
    void handle(const LogRequest& request);
    void handle(const SetLevelCommand& command);

    logger::ILogger& log_;
    std::ostream& errors_;

    std::atomic<std::size_t> processed_{0};
    std::atomic<std::size_t> failed_{0};
    std::atomic<std::size_t> dropped_{0};

    BlockingQueue<QueueItem> queue_;
    std::thread worker_; 
};

}  // namespace app
