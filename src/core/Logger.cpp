#include "core/Logger.hpp"

#include <iostream>
#include <ostream>

namespace traffic::core {

Logger::Logger(std::ostream& output)
    : output_(&output) {}

void Logger::set_level(LogLevel level) noexcept {
    level_ = level;
}

LogLevel Logger::level() const noexcept {
    return level_;
}

void Logger::set_output(std::ostream& output) noexcept {
    std::scoped_lock lock(mutex_);
    output_ = &output;
}

std::ostream& Logger::output() const noexcept {
    return *output_;
}

void Logger::log(LogLevel level, std::string_view message) {
    if (static_cast<int>(level) < static_cast<int>(level_)) {
        return;
    }

    write(level, message);
}

void Logger::trace(std::string_view message) {
    log(LogLevel::Trace, message);
}

void Logger::debug(std::string_view message) {
    log(LogLevel::Debug, message);
}

void Logger::info(std::string_view message) {
    log(LogLevel::Info, message);
}

void Logger::warn(std::string_view message) {
    log(LogLevel::Warn, message);
}

void Logger::error(std::string_view message) {
    log(LogLevel::Error, message);
}

constexpr std::string_view Logger::level_name(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::Trace:
        return "TRACE";
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    }

    return "INFO";
}

void Logger::write(LogLevel level, std::string_view message) {
    std::scoped_lock lock(mutex_);
    if (output_ == nullptr) {
        return;
    }

    (*output_) << '[' << level_name(level) << "] " << message << '\n';
    output_->flush();
}

Logger& default_logger() {
    static Logger logger {std::clog};
    return logger;
}

}  // namespace traffic::core
