#pragma once

#include "pch.h"

struct IngredientInfo
{
    RE::FormID    formID{ 0 };
    std::string   name;
    std::uint32_t baseNeed{ 0 };
    std::uint32_t need{ 0 };
    std::uint32_t have{ 0 };
    std::uint32_t missing{ 0 };
};

struct TrackedRecipe
{
    RE::FormID                  cobjFormID{ 0 };
    RE::FormID                  createdFormID{ 0 };
    std::string                 itemName;
    std::uint32_t               countProduced{ 1 };
    std::uint32_t               multiplier{ 1 };
    std::vector<IngredientInfo> ingredients;
    bool                        isReady{ false };
};
