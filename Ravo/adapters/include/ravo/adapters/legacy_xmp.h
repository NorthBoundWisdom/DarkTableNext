#pragma once

#include <string_view>

#include "ravo/foundation/error.h"
#include "ravo/recipe/recipe.h"

namespace ravo
{

struct LegacyXmpImportRequest
{
    std::string_view xmp_utf8;
    AssetDescriptor asset;
};

// Parses a bounded legacy XMP document without loading the frozen module ABI.
[[nodiscard]] Result<Recipe> import_legacy_xmp(const LegacyXmpImportRequest &request);

} // namespace ravo
