#pragma once

#include <chrono>
#include <string>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>

namespace Logger {
    namespace {
        // Get current system time as a string
        std::string getCurrentTime() {
            const auto currentTime = std::chrono::system_clock::now();
            return fmt::format("{:%T}", fmt::localtime(std::chrono::system_clock::to_time_t(currentTime)));
        }

        template<typename... Args>
        void print(std::string_view logTag, fmt::text_style colour, fmt::format_string<Args...> formatString, Args&&... args) {
            fmt::print(
                stderr,
                colour,
                "[{}] ({}) - {}\n",
                logTag,
                Logger::getCurrentTime(),
                fmt::format(formatString, std::forward<Args>(args)...)
            );
        }
    }

    // Logs a message which is useful for debugging and isn't relevant to the user
    template<typename... Args>
    void debug(fmt::format_string<Args...> formatString, Args&&... args) {
        Logger::print("DEBUG", fg(fmt::color::light_golden_rod_yellow), formatString, std::forward<Args>(args)...);
    };

    // Logs a general message which informs the user about the state of the program
    template<typename... Args>
    void info(fmt::format_string<Args...> formatString, Args&&... args) {
        Logger::print("INFO", fg(fmt::color::powder_blue), formatString, std::forward<Args>(args)...);
    };

    // Logs a warning message which indicates that something may have gone wrong but isn't necessarily an error
    template<typename... Args>
    void warn(fmt::format_string<Args...> formatString, Args&&... args) {
        Logger::print("WARN", fg(fmt::color::orange), formatString, std::forward<Args>(args)...);
    };

    // Logs an error message which indicates that something has gone wrong during program execution
    // May or may not lead to program exit
    template<typename... Args>
    void err(fmt::format_string<Args...> formatString, Args&&... args) {
        Logger::print("ERROR", fg(fmt::color::red), formatString, std::forward<Args>(args)...);
    };
}