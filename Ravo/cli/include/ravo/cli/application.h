#pragma once

#include <iosfwd>
#include <span>
#include <string_view>

#include "ravo/engine/engine.h"

namespace ravo
{

class CliApplication
{
public:
    // The caller owns the engine and both streams for the application's lifetime.
    CliApplication(const EngineFacade &engine, std::ostream &stdout_stream,
                   std::ostream &stderr_stream);

    [[nodiscard]] int run(std::span<const std::string_view> arguments) const;

private:
    const EngineFacade &engine_;
    std::ostream &stdout_stream_;
    std::ostream &stderr_stream_;
};

} // namespace ravo
