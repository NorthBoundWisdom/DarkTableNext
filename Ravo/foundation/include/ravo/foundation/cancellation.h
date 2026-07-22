#pragma once

#include <memory>
#include <string>

#include "ravo/foundation/error.h"

namespace ravo
{

namespace detail
{
struct CancellationState;
}

class CancellationSource;

class CancellationToken
{
public:
    CancellationToken() = default;

    [[nodiscard]] bool is_cancellation_requested() const noexcept;
    [[nodiscard]] std::string reason() const;
    [[nodiscard]] Result<void> check() const;

private:
    explicit CancellationToken(std::shared_ptr<detail::CancellationState> state);

    std::shared_ptr<detail::CancellationState> state_;

    friend class CancellationSource;
};

class CancellationSource
{
public:
    CancellationSource();

    [[nodiscard]] CancellationToken token() const;
    [[nodiscard]] bool cancel(std::string reason = "cancelled");

private:
    std::shared_ptr<detail::CancellationState> state_;
};

} // namespace ravo
