#include "ravo/foundation/error.h"

namespace ravo
{

std::string_view error_code_name(const ErrorCode code) noexcept
{
    switch (code)
    {
    case ErrorCode::kInternal:
        return "internal";
    case ErrorCode::kInvalidArgument:
        return "invalid_argument";
    case ErrorCode::kNotFound:
        return "not_found";
    case ErrorCode::kValidation:
        return "validation";
    case ErrorCode::kUnsupported:
        return "unsupported";
    case ErrorCode::kIo:
        return "io";
    case ErrorCode::kCancelled:
        return "cancelled";
    case ErrorCode::kConflict:
        return "conflict";
    }
    return "internal";
}

int cli_exit_code(const ErrorCode code) noexcept
{
    switch (code)
    {
    case ErrorCode::kInvalidArgument:
        return 2;
    case ErrorCode::kNotFound:
        return 3;
    case ErrorCode::kValidation:
        return 4;
    case ErrorCode::kUnsupported:
        return 5;
    case ErrorCode::kIo:
    case ErrorCode::kConflict:
        return 6;
    case ErrorCode::kCancelled:
        return 7;
    case ErrorCode::kInternal:
        return 70;
    }
    return 70;
}

TaskError make_error(const ErrorCode code, std::string message,
                     std::map<std::string, std::string, std::less<>> context)
{
    return TaskError{code, std::move(message), std::move(context)};
}

} // namespace ravo
