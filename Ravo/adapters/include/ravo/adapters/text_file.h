#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "ravo/foundation/error.h"

namespace ravo
{

// The adapter owns native path encoding; callers supply an unretained UTF-8 path.
[[nodiscard]] Result<std::string> read_utf8_text_file(std::string_view path_utf8,
                                                      std::uintmax_t maximum_bytes = 1024U * 1024U);

// Writes one complete UTF-8 byte sequence or leaves the prior target untouched.
[[nodiscard]] Result<void> write_utf8_text_file_atomically(std::string_view path_utf8,
                                                           std::string_view content_utf8);

} // namespace ravo
