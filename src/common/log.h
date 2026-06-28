#pragma once

#include <chrono>
#include <string>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/std.h>

namespace log {
    namespace internal {
        // Get current system time as a string
        inline std::string getCurrentTime() {
            const auto nowUtc = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
            const auto t = std::chrono::system_clock::to_time_t(nowUtc);
            return fmt::format("{:%T}", *std::localtime(&t));
        }

        // Logs a message with category, colour and timestamp
        template<typename... Args>
        void print(std::string_view logTag, fmt::text_style colour, fmt::format_string<Args...> formatString, Args&&... args) {
            fmt::print(
                stderr,
                colour,
                "[{}] ({}) - {}\n",
                logTag,
                getCurrentTime(),
                fmt::format(formatString, std::forward<Args>(args)...)
            );
        }
    }

    // Logs a message which is useful for debugging and isn't relevant to the user
    template<typename... Args>
    void debug(fmt::format_string<Args...> formatString, Args&&... args) {
        log::internal::print("DEBUG", fg(fmt::color::light_golden_rod_yellow), formatString, std::forward<Args>(args)...);
    }

    // Logs a general message which informs the user about the state of the program
    template<typename... Args>
    void info(fmt::format_string<Args...> formatString, Args&&... args) {
        log::internal::print("INFO", fg(fmt::color::powder_blue), formatString, std::forward<Args>(args)...);
    }

    // Logs a warning message which indicates that something may have gone wrong but isn't necessarily an error
    template<typename... Args>
    void warn(fmt::format_string<Args...> formatString, Args&&... args) {
        log::internal::print("WARN", fg(fmt::color::orange), formatString, std::forward<Args>(args)...);
    }

    // Logs an error message which indicates that something has gone wrong during program execution.
    // May or may not lead to program exit.
    template<typename... Args>
    void err(fmt::format_string<Args...> formatString, Args&&... args) {
        log::internal::print("ERROR", fg(fmt::color::orange_red), formatString, std::forward<Args>(args)...);
    }

    // Logs an error message which indicates that something has gone massively wrong during program execution.
    // Should lead to program exit.
    template<typename... Args>
    void fatal(fmt::format_string<Args...> formatString, Args&&... args) {
        log::internal::print("FATAL", fg(fmt::color::crimson), formatString, std::forward<Args>(args)...);
    }
}
