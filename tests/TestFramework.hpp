#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace testing {

struct Failure {
    std::string file;
    int line;
    std::string message;
};

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

int runAll();

}

namespace testing {

template <typename T>
std::string describe(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

inline std::string describe(bool value) { return value ? "true" : "false"; }

}  

#define TEST(test_name)                                                     \
    static void test_name(testing::TestContext& ctx_);                      \
    static const testing::Registrar registrar_##test_name(#test_name,       \
                                                          test_name);       \
    static void test_name([[maybe_unused]] testing::TestContext& ctx_)


#define CHECK(condition)                                                    \
    do {                                                                    \
        if (!(condition)) {                                                 \
            ctx_.addFailure(__FILE__, __LINE__,                             \
                            "условие ложно: " #condition);                  \
        }                                                                   \
    } while (false)

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

#define CHECK_THROWS_AS(expression, exception_type)                         \
    do {                                                                    \
        bool thrown_ = false;                                               \
        try {                                                               \
            (void)(expression);                                             \
        } catch (const exception_type&) {                                   \
            thrown_ = true;                                                 \
        } catch (...) {                                                     \
            ctx_.addFailure(__FILE__, __LINE__,                             \
                            "брошено исключение неожиданного типа: "        \
                            #expression);                                   \
            break;                                                          \
        }                                                                   \
        if (!thrown_) {                                                     \
            ctx_.addFailure(__FILE__, __LINE__,                             \
                            "ожидалось исключение " #exception_type         \
                            " от: " #expression);                           \
        }                                                                   \
    } while (false)

#define CHECK_NO_THROW(expression)                                          \
    do {                                                                    \
        try {                                                               \
            (void)(expression);                                             \
        } catch (...) {                                                     \
            ctx_.addFailure(__FILE__, __LINE__,                             \
                            "неожиданное исключение от: " #expression);     \
        }                                                                   \
    } while (false)
