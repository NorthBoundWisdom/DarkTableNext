#include "ravo/foundation/cancellation.h"

#include <atomic>
#include <mutex>
#include <utility>

namespace ravo::detail
{

struct CancellationState
{
    std::atomic_bool requested = false;
    mutable std::mutex mutex;
    std::string reason;
};

} // namespace ravo::detail

namespace ravo
{

CancellationToken::CancellationToken(std::shared_ptr<detail::CancellationState> state)
    : state_(std::move(state))
{
}

bool CancellationToken::is_cancellation_requested() const noexcept
{
    return state_ != nullptr && state_->requested.load(std::memory_order_acquire);
}

std::string CancellationToken::reason() const
{
    if (state_ == nullptr)
    {
        return {};
    }
    std::lock_guard lock(state_->mutex);
    return state_->reason;
}

Result<void> CancellationToken::check() const
{
    if (!is_cancellation_requested())
    {
        return {};
    }

    return make_error(ErrorCode::kCancelled, "Ravo task was cancelled", {{"reason", reason()}});
}

CancellationSource::CancellationSource()
    : state_(std::make_shared<detail::CancellationState>())
{
}

CancellationToken CancellationSource::token() const
{
    return CancellationToken{state_};
}

bool CancellationSource::cancel(std::string reason)
{
    bool expected = false;
    if (!state_->requested.compare_exchange_strong(expected, true, std::memory_order_release,
                                                   std::memory_order_relaxed))
    {
        return false;
    }

    std::lock_guard lock(state_->mutex);
    state_->reason = std::move(reason);
    return true;
}

} // namespace ravo
