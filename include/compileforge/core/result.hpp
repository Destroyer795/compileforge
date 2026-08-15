#ifndef COMPILEFORGE_CORE_RESULT_HPP
#define COMPILEFORGE_CORE_RESULT_HPP

#include <string>
#include <variant>
#include <utility>
#include <stdexcept>
#include <optional>

namespace compileforge {

enum class ErrorCode {
    Success = 0,
    FileNotFound,
    InvalidPath,
    ParseError,
    InvalidCompilationDatabase,
    GraphCycleDetected,
    GitError,
    CacheError,
    ConfigError,
    IoError,
    UnknownError
};

struct Error {
    ErrorCode code{ErrorCode::UnknownError};
    std::string message;
    std::string location;

    Error(ErrorCode c, std::string msg, std::string loc = "")
        : code(c), message(std::move(msg)), location(std::move(loc)) {}

    std::string to_string() const {
        std::string code_str;
        switch (code) {
            case ErrorCode::FileNotFound: code_str = "FileNotFound"; break;
            case ErrorCode::InvalidPath: code_str = "InvalidPath"; break;
            case ErrorCode::ParseError: code_str = "ParseError"; break;
            case ErrorCode::InvalidCompilationDatabase: code_str = "InvalidCompilationDatabase"; break;
            case ErrorCode::GraphCycleDetected: code_str = "GraphCycleDetected"; break;
            case ErrorCode::GitError: code_str = "GitError"; break;
            case ErrorCode::CacheError: code_str = "CacheError"; break;
            case ErrorCode::ConfigError: code_str = "ConfigError"; break;
            case ErrorCode::IoError: code_str = "IoError"; break;
            default: code_str = "UnknownError"; break;
        }
        if (location.empty()) {
            return "[" + code_str + "] " + message;
        }
        return "[" + code_str + " at " + location + "] " + message;
    }
};

template <typename T>
class Result {
public:
    Result(T val) : storage_(std::move(val)) {}
    Result(Error err) : storage_(std::move(err)) {}

    [[nodiscard]] bool has_value() const noexcept {
        return std::holds_alternative<T>(storage_);
    }

    [[nodiscard]] bool is_ok() const noexcept {
        return has_value();
    }

    [[nodiscard]] bool is_error() const noexcept {
        return std::holds_alternative<Error>(storage_);
    }

    [[nodiscard]] const T& value() const {
        if (!has_value()) {
            throw std::runtime_error("Attempted to access value of error Result: " + error().to_string());
        }
        return std::get<T>(storage_);
    }

    [[nodiscard]] T& value() {
        if (!has_value()) {
            throw std::runtime_error("Attempted to access value of error Result: " + error().to_string());
        }
        return std::get<T>(storage_);
    }

    [[nodiscard]] const Error& error() const {
        if (!is_error()) {
            throw std::runtime_error("Attempted to access error of successful Result");
        }
        return std::get<Error>(storage_);
    }

    [[nodiscard]] T value_or(T default_val) const {
        if (has_value()) {
            return std::get<T>(storage_);
        }
        return default_val;
    }

    template <typename Fn>
    auto map(Fn&& fn) const -> Result<decltype(fn(std::declval<T>()))> {
        using ReturnType = decltype(fn(std::declval<T>()));
        if (has_value()) {
            return Result<ReturnType>(fn(std::get<T>(storage_)));
        }
        return Result<ReturnType>(error());
    }

private:
    std::variant<T, Error> storage_;
};

template <>
class Result<void> {
public:
    Result() : error_(std::nullopt) {}
    Result(Error err) : error_(std::move(err)) {}

    [[nodiscard]] bool has_value() const noexcept {
        return !error_.has_value();
    }

    [[nodiscard]] bool is_error() const noexcept {
        return error_.has_value();
    }

    [[nodiscard]] const Error& error() const {
        if (!is_error()) {
            throw std::runtime_error("Attempted to access error of successful Result<void>");
        }
        return *error_;
    }

private:
    std::optional<Error> error_;
};

} // namespace compileforge

#endif // COMPILEFORGE_CORE_RESULT_HPP
