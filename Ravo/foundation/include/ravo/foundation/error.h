#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace ravo
{

enum class ErrorCode
{
    kInternal,
    kInvalidArgument,
    kNotFound,
    kValidation,
    kUnsupported,
    kIo,
    kCancelled,
    kConflict,
};

struct TaskError
{
    ErrorCode code = ErrorCode::kInternal;
    std::string message;
    std::map<std::string, std::string, std::less<>> context;
};

[[nodiscard]] std::string_view error_code_name(ErrorCode code) noexcept;
[[nodiscard]] int cli_exit_code(ErrorCode code) noexcept;

[[nodiscard]] TaskError make_error(ErrorCode code, std::string message,
                                   std::map<std::string, std::string, std::less<>> context = {});

template <typename T>
class [[nodiscard]] Result
{
public:
    Result(T value)
        : value_(std::move(value))
    {
    }
    Result(TaskError error)
        : value_(std::move(error))
    {
    }

    [[nodiscard]] bool has_value() const noexcept
    {
        return std::holds_alternative<T>(value_);
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return has_value();
    }

    [[nodiscard]] T &value() &
    {
        return std::get<T>(value_);
    }
    [[nodiscard]] const T &value() const &
    {
        return std::get<T>(value_);
    }
    [[nodiscard]] T &&value() &&
    {
        return std::get<T>(std::move(value_));
    }

    [[nodiscard]] TaskError &error() &
    {
        return std::get<TaskError>(value_);
    }
    [[nodiscard]] const TaskError &error() const &
    {
        return std::get<TaskError>(value_);
    }

private:
    std::variant<T, TaskError> value_;
};

template <>
class [[nodiscard]] Result<void>
{
public:
    Result() = default;
    Result(TaskError error)
        : error_(std::move(error))
    {
    }

    [[nodiscard]] bool has_value() const noexcept
    {
        return !error_.has_value();
    }
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return has_value();
    }
    void value() const noexcept
    {
    }
    [[nodiscard]] TaskError &error() &
    {
        return *error_;
    }
    [[nodiscard]] const TaskError &error() const &
    {
        return *error_;
    }

private:
    std::optional<TaskError> error_;
};

} // namespace ravo
