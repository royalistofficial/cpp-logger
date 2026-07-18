#include "TestFramework.hpp"

#include <iostream>

namespace testing {

int runAll() {
    const auto& entries = Registry::instance().entries();
    std::size_t failed = 0;

    for (const auto& entry : entries) {
        TestContext ctx;
        entry.body(ctx);

        if (ctx.passed()) {
            std::cout << "[  OK  ] " << entry.name << '\n';
        } else {
            ++failed;
            std::cout << "[ FAIL ] " << entry.name << '\n';
            for (const Failure& failure : ctx.failures()) {
                std::cout << "         " << failure.file << ':' << failure.line
                          << ": " << failure.message << '\n';
            }
        }
    }

    std::cout << "\nВсего тестов: " << entries.size()
              << ", провалено: " << failed << '\n';
    return failed == 0 ? 0 : 1;
}

}  // namespace testing

int main() {
    return testing::runAll();
}
