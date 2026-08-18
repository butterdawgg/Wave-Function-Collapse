#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <iostream>
#include <format>



namespace ANSI_Colors
{
    constexpr const char* MAYBE_RED = "\033[33m";
    constexpr const char* JUST_RED = "\033[31m";
    constexpr const char* VERY_RED = "\033[30;101m";
    constexpr const char* RESET = "\033[0m";
}

template<typename... Args>
inline void logMsg(std::format_string<Args...> fmt, Args&&... args)
{
    std::cerr << "LOG: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';
}

template<typename... Args>
inline void logWarning(std::format_string<Args...> fmt, Args&&... args)
{
    std::cerr << ANSI_Colors::MAYBE_RED
        << "WARNING: "
        << std::format(fmt, std::forward<Args>(args)...)
        << ANSI_Colors::RESET
        << '\n';
}

template<typename... Args>
inline void logError(std::format_string<Args...> fmt, Args&&... args)
{
    std::cerr << ANSI_Colors::JUST_RED
        << "ERROR: "
        << std::format(fmt, std::forward<Args>(args)...)
        << ANSI_Colors::RESET
        << '\n';
}

template<typename... Args>
inline void logCaughtException(std::format_string<Args...> fmt, Args&&... args)
{
    std::cerr << ANSI_Colors::VERY_RED
        << "EXCEPTION CAUGHT: "
        << std::format(fmt, std::forward<Args>(args)...)
        << ANSI_Colors::RESET
        << '\n';
}

#endif