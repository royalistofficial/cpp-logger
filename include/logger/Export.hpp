#pragma once

/**
 * @file
 * @brief Управление видимостью символов динамической библиотеки.
 */
#if defined(__GNUC__) || defined(__clang__)
#define LOGGER_API __attribute__((visibility("default")))
#else
#define LOGGER_API
#endif
