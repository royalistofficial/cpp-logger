#include "logger/LogLevel.hpp"

#include <iostream>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    std::cout << logger::toString(logger::LogLevel::Warning) << '\n';
    return 0;
}