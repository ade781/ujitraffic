#pragma once

#include <iosfwd>
#include <mutex>
#include <string_view>

namespace traffic::core {

enum class LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
};

class Logger {
public:
    explicit Logger(std::ostream& output);

    void set_level(LogLevel level) noexcept;
    LogLevel level() const noexcept;

    void set_output(std::ostream& output) noexcept;
    std::ostream& output() const noexcept;

    void log(LogLevel level, std::string_view message);
    void trace(std::string_view message);
    void debug(std::string_view message);
    void info(std::string_view message);
    void warn(std::string_view message);
    void error(std::string_view message);

private:
    static constexpr std::string_view level_name(LogLevel level) noexcept;

    void write(LogLevel level, std::string_view message);

    std::ostream* output_ {nullptr};
    LogLevel level_ {LogLevel::Info};
    mutable std::mutex mutex_;
};

Logger& default_logger();

}  // namespace traffic::core

