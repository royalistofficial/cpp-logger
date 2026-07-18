#include "TestFramework.hpp"

#include "BlockingQueue.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <optional>
#include <future>
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

}  // namespace

TEST(ЭлементыВыдаютсяВПорядкеДобавления) {
    BlockingQueue<int> queue;

    CHECK(queue.push(1));
    CHECK(queue.push(2));
    CHECK(queue.push(3));

    CHECK_EQ(queue.pop().value_or(-1), 1);
    CHECK_EQ(queue.pop().value_or(-1), 2);
    CHECK_EQ(queue.pop().value_or(-1), 3);
}

TEST(PopЖдётПоявленияЭлемента) {
    BlockingQueue<std::string> queue;

    auto consumer = std::async(std::launch::async, [&queue] {
        return queue.pop();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    queue.push("привет");

    CHECK(waitFor(consumer));
    CHECK_EQ(consumer.get().value_or(std::string()), std::string("привет"));
}

TEST(ЗакрытиеБудитОжидающегоПотребителя) {
    BlockingQueue<int> queue;

    auto consumer = std::async(std::launch::async, [&queue] {
        return queue.pop().has_value();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    queue.close();

    CHECK(waitFor(consumer));
    CHECK(!consumer.get());
}

TEST(ПослеЗакрытияОстаткиДочитываются) {
    BlockingQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.close();

    CHECK_EQ(queue.pop().value_or(-1), 1);
    CHECK_EQ(queue.pop().value_or(-1), 2);
    CHECK(!queue.pop().has_value());
}

TEST(PushВЗакрытуюОчередьОтвергается) {
    BlockingQueue<int> queue;
    queue.close();

    CHECK(!queue.push(1));
    CHECK_EQ(queue.size(), static_cast<std::size_t>(0));
    CHECK(queue.isClosed());
}

TEST(ЁмкостьОграничиваетРазмерОчереди) {
    BlockingQueue<int> queue(2);

    CHECK(queue.push(1));
    CHECK(queue.push(2));
    CHECK_EQ(queue.size(), static_cast<std::size_t>(2));

    // Третий push обязан ждать: очередь заполнена.
    auto producer = std::async(std::launch::async, [&queue] {
        return queue.push(3);
    });

    CHECK(!waitFor(producer, 50));
    CHECK_EQ(queue.pop().value_or(-1), 1);
    CHECK(waitFor(producer));
    CHECK(producer.get());
    CHECK_EQ(queue.size(), static_cast<std::size_t>(2));
}

TEST(ЗакрытиеБудитОжидающегоПроизводителя) {
    BlockingQueue<int> queue(1);
    queue.push(1);

    auto producer = std::async(std::launch::async, [&queue] {
        return queue.push(2);
    });

    CHECK(!waitFor(producer, 50));
    queue.close();

    CHECK(waitFor(producer));
    CHECK(!producer.get());  // элемент отброшен, а не потерян молча
}

TEST(НулеваяЁмкостьОзначаетБезОграничения) {
    BlockingQueue<int> queue(BlockingQueue<int>::kUnbounded);

    for (int i = 0; i < 1000; ++i) {
        CHECK(queue.push(i));
    }
    CHECK_EQ(queue.size(), static_cast<std::size_t>(1000));
    CHECK_EQ(queue.capacity(), static_cast<std::size_t>(0));
}

TEST(МногиеПроизводителиИПотребителиНеТеряютЭлементов) {
    BlockingQueue<int> queue(16);

    constexpr int kProducers = 4;
    constexpr int kPerProducer = 500;
    std::atomic<int> consumed{0};
    std::atomic<long long> sum{0};

    std::vector<std::thread> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.emplace_back([&queue, &consumed, &sum] {
            while (std::optional<int> value = queue.pop()) {
                sum.fetch_add(*value);
                consumed.fetch_add(1);
            }
        });
    }

    std::vector<std::thread> producers;
    for (int p = 0; p < kProducers; ++p) {
        producers.emplace_back([&queue] {
            for (int i = 0; i < kPerProducer; ++i) {
                queue.push(1);
            }
        });
    }

    for (std::thread& producer : producers) {
        producer.join();
    }
    queue.close();
    for (std::thread& consumer : consumers) {
        consumer.join();
    }

    CHECK_EQ(consumed.load(), kProducers * kPerProducer);
    CHECK_EQ(sum.load(), static_cast<long long>(kProducers * kPerProducer));
}
