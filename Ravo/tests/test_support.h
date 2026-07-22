#pragma once

#include "ravo/recipe/recipe.h"

namespace ravo::test
{

inline Recipe valid_recipe()
{
    Recipe recipe;
    recipe.asset = {"asset-1", "file:///fixture.raw", std::nullopt};
    recipe.operations.push_back({"ravo.core.exposure",
                                 1,
                                 "exposure-1",
                                 true,
                                 {{"exposure_ev", ParameterValue{1.25}}},
                                 std::nullopt});
    return recipe;
}

} // namespace ravo::test
