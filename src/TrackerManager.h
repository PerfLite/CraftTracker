#pragma once

#include "pch.h"
#include "TrackerTypes.h"
#include <functional>

class TrackerManager
{
public:
    static TrackerManager* GetSingleton()
    {
        static TrackerManager singleton;
        return &singleton;
    }

    bool ToggleRecipe(RE::BGSConstructibleObject* a_cobj);
    bool AddRecipe(RE::BGSConstructibleObject* a_cobj, std::uint32_t a_multiplier = 1);
    std::uint32_t CycleRecipeMultiplier(RE::BGSConstructibleObject* a_cobj);
    bool UntrackRecipe(RE::FormID a_cobjFormID);
    bool OnItemCrafted(RE::FormID a_createdFormID);
    void Clear();

    bool IsTracked(RE::FormID a_cobjFormID) const;
    bool NeedsIngredient(RE::FormID a_baseFormID) const;

    void RefreshInventory(bool a_notifyUI = true);

    std::string GetJsonState() const;
    std::size_t GetTrackedCount() const;
    std::vector<RE::FormID> GetTrackedRecipeIDs() const;

    void SetOnStateChanged(std::function<void(const std::string&)> a_callback)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_onStateChanged = std::move(a_callback);
    }

    void NotifyStateChanged() const;

private:
    TrackerManager() = default;
    ~TrackerManager() = default;
    TrackerManager(const TrackerManager&) = delete;
    TrackerManager& operator=(const TrackerManager&) = delete;

    void RebuildNeededIngredients();

    mutable std::recursive_mutex            m_mutex;
    std::vector<TrackedRecipe>              m_recipes;
    std::unordered_set<RE::FormID>          m_neededIngredients;
    std::function<void(const std::string&)> m_onStateChanged;
};
