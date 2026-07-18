#pragma once

// Минимальный тестовый фреймворк: регистрация тестов через статические
// объекты, запуск из test_main.cpp. Сторонние библиотеки условиями задания
// запрещены, поэтому реализация умышленно предельно простая.

#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace testing {

/// Одна проваленная проверка внутри теста.
struct Failure {
    std::string file;
    int line;
    std::string message;
};

/// Контекст выполняемого теста: собирает провалы, не прерывая тест.
class TestContext {
public:
    void addFailure(const char* file, int line, std::string message) {
        failures_.push_back({file, line, std::move(message)});
    }

    const std::vector<Failure>& failures() const noexcept { return failures_; }
    bool passed() const noexcept { return failures_.empty(); }

private:
    std::vector<Failure> failures_;
};

/// Глобальный реестр тестов.
class Registry {
public:
    using TestBody = std::function<void(TestContext&)>;

    struct Entry {
        std::string name;
        TestBody body;
    };

    static Registry& instance() {
        static Registry registry;
        return registry;
    }

    void add(std::string name, TestBody body) {
        entries_.push_back({std::move(name), std::move(body)});
    }

    const std::vector<Entry>& entries() const noexcept { return entries_; }

private:
    std::vector<Entry> entries_;
};

struct Registrar {
    Registrar(std::string name, Registry::TestBody body) {
        Registry::instance().add(std::move(name), std::move(body));
    }
};

/// Запускает все зарегистрированные тесты. Возвращает код выхода процесса.
int runAll();

}  // namespace testing

namespace testing {

template <typename T>
std::string describe(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

inline std::string describe(bool value) { return value ? "true" : "false"; }

}  // namespace testing

/// Объявление теста: TEST(имя) { ... }
#define TEST(test_name)                                                     \
    static void test_name(testing::TestContext& ctx_);                      \
    static const testing::Registrar registrar_##test_name(#test_name,       \
                                                          test_name);       \
    static void test_name([[maybe_unused]] testing::TestContext& ctx_)

/// Проверка истинности
#define CHECK(condition)                                                    \
    do {                                                                    \
        if (!(condition)) {                                                 \
            ctx_.addFailure(__FILE__, __LINE__,                             \
                            "условие ложно: " #condition);                  \
        }                                                                   \
    } while (false)

/// Проверка равенства
#define CHECK_EQ(actual, expected)                                          \
    do {                                                                    \
        const auto actual_value_ = (actual);                                \
        const auto expected_value_ = (expected);                            \
        if (!(actual_value_ == expected_value_)) {                          \
            std::ostringstream oss_;                                        \
            oss_ << #actual " == " #expected " — получено: "                \
                 << testing::describe(actual_value_)                        \
                 << ", ожидалось: " << testing::describe(expected_value_);  \
            ctx_.addFailure(__FILE__, __LINE__, oss_.str());                \
        }                                                                   \
    } while (false)