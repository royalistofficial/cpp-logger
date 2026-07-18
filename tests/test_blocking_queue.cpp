#include "TestFramework.hpp"

#include "BlockingQueue.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using app::BlockingQueue;

namespace {

template <typename Future>
bool waitFor(Future& future, int milliseconds = 2000) {
    return future.wait_for(std::chrono::milliseconds(milliseconds)) ==
           std::future_status::ready;
}

} 

TEST(ЭлементыВыдаютсяВПорядкеДобавления) {
    BlockingQueue<int> queue;

    CHECK(queue.push(1));
    CHECK(queue.push(2));
    CHECK(queue.push(3));
    CHECK_EQ(queue.size(), std::size_t(3));

    CHECK_EQ(queue.pop().value(), 1);
    CHECK_EQ(queue.pop().value(), 2);
    CHECK_EQ(queue.pop().value(), 3);
    CHECK_EQ(queue.size(), std::size_t(0));
}

TEST(PopЖдётПоявленияЭлемента) {
    BlockingQueue<std::string> queue;

    std::future<std::optional<std::string>> consumer =
        std::async(std::launch::async, [&queue] { return queue.pop(); });

    CHECK(consumer.wait_for(std::chrono::milliseconds(50)) ==
          std::future_status::timeout);

    queue.push("сообщение");

    CHECK(waitFor(consumer));
    const std::optional<std::string> result = consumer.get();
    CHECK(result.has_value());
    CHECK_EQ(result.value(), std::string("сообщение"));
}

TEST(CloseБудитОжидающегоПотребителя) {
    BlockingQueue<int> queue;

    std::future<std::optional<int>> consumer =
        std::async(std::launch::async, [&queue] { return queue.pop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.close();

    const bool finished = waitFor(consumer);
    CHECK(finished);

    if (!finished) {
        std::cout.flush();
        return;
    }

    CHECK(!consumer.get().has_value());
}

TEST(НакопленныеЭлементыВыдаютсяПослеЗакрытия) {
    BlockingQueue<int> queue;

    queue.push(10);
    queue.push(20);
    queue.close();

    CHECK_EQ(queue.pop().value(), 10);
    CHECK_EQ(queue.pop().value(), 20);
    CHECK(!queue.pop().has_value());
}

TEST(ДобавлениеПослеЗакрытияОтвергается) {
    BlockingQueue<int> queue;
    queue.close();

    CHECK(!queue.push(1));
    CHECK_EQ(queue.size(), std::size_t(0));
    CHECK(queue.isClosed());
}

TEST(ПовторноеЗакрытиеБезопасно) {
    BlockingQueue<int> queue;

    queue.close();
    queue.close();

    CHECK(queue.isClosed());
    CHECK(!queue.pop().has_value());
}

TEST(ВсеПотребителиЗавершаютсяПоЗакрытию) {
    BlockingQueue<int> queue;
    constexpr int kConsumers = 4;

    std::vector<std::future<std::optional<int>>> consumers;
    consumers.reserve(kConsumers);

    for (int i = 0; i < kConsumers; ++i) {
        consumers.push_back(
            std::async(std::launch::async, [&queue] { return queue.pop(); }));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.close();

    for (std::future<std::optional<int>>& consumer : consumers) {
        const bool finished = waitFor(consumer);
        CHECK(finished);
        if (!finished) {
            std::cout.flush();
            return;
        }
        CHECK(!consumer.get().has_value());
    }
}

TEST(НесколькоПоставщиковИПотребителей) {
    BlockingQueue<int> queue;

    constexpr int kProducers = 4;
    constexpr int kConsumers = 3;
    constexpr int kItemsPerProducer = 1000;

    std::atomic<int> consumed{0};
    std::atomic<long long> sum{0};

    std::vector<std::thread> workers;

    for (int c = 0; c < kConsumers; ++c) {
        workers.emplace_back([&queue, &consumed, &sum] {
            while (std::optional<int> item = queue.pop()) {
                sum += *item;
                ++consumed;
            }
        });
    }

    for (int p = 0; p < kProducers; ++p) {
        workers.emplace_back([&queue, p] {
            for (int i = 0; i < kItemsPerProducer; ++i) {
                queue.push(p * kItemsPerProducer + i);
            }
        });
    }

    for (std::size_t i = kConsumers; i < workers.size(); ++i) {
        workers[i].join();
    }

    queue.close();

    for (int c = 0; c < kConsumers; ++c) {
        workers[static_cast<std::size_t>(c)].join();
    }

    constexpr int kTotal = kProducers * kItemsPerProducer;
    constexpr long long kExpectedSum =
        static_cast<long long>(kTotal) * (kTotal - 1) / 2;

    CHECK_EQ(consumed.load(), kTotal);
    CHECK_EQ(sum.load(), kExpectedSum);
    CHECK_EQ(queue.size(), std::size_t(0));
}

TEST(ОчередьПеремещаетНеКопируемыеТипы) {
    BlockingQueue<std::unique_ptr<int>> queue;

    queue.push(std::make_unique<int>(42));

    const std::optional<std::unique_ptr<int>> item = queue.pop();
    CHECK(item.has_value());
    CHECK_EQ(**item, 42);
}
