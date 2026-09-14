#include "pch.h"
#include "TrackerManager.h"
#include "JsonHelper.h"
#include "DebugLog.h"
#include "Config.h"
#include "RE/S/SendHUDMessage.h"

bool TrackerManager::ToggleRecipe(RE::BGSConstructibleObject* a_cobj)
{
    if (!a_cobj) return false;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    const auto formID = a_cobj->GetFormID();

    auto it = std::find_if(m_recipes.begin(), m_recipes.end(),
        [formID](const TrackedRecipe& r) { return r.cobjFormID == formID; });

    if (it != m_recipes.end()) {
        std::string name = it->itemName;
        m_recipes.erase(it);
        RebuildNeededIngredients();
        Dbg("Untracked recipe: " + name + " (0x" + std::to_string(formID) + ")");
        NotifyStateChanged();
        return false; // Removed
    }

    return AddRecipe(a_cobj);
}

bool TrackerManager::AddRecipe(RE::BGSConstructibleObject* a_cobj, std::uint32_t a_multiplier)
{
    if (!a_cobj) return false;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    const auto formID = a_cobj->GetFormID();

    // Check if already exists
    for (const auto& r : m_recipes) {
        if (r.cobjFormID == formID) {
            return false;
        }
    }

    TrackedRecipe recipe;
    recipe.cobjFormID = formID;
    recipe.multiplier = (a_multiplier > 0) ? a_multiplier : 1;

    if (a_cobj->createdItem) {
        recipe.createdFormID = a_cobj->createdItem->GetFormID();
        recipe.itemName = a_cobj->createdItem->GetName();
    }
    if (recipe.itemName.empty()) {
        recipe.itemName = a_cobj->GetName();
    }
    if (recipe.itemName.empty()) {
        recipe.itemName = "Неизвестный предмет";
    }

    recipe.countProduced = a_cobj->data.numConstructed > 0 ? a_cobj->data.numConstructed : 1;

    // Collect required ingredients
    a_cobj->requiredItems.ForEachContainerObject([&](RE::ContainerObject& entry) {
        if (entry.obj && entry.count > 0) {
            IngredientInfo info;
            info.formID = entry.obj->GetFormID();
            info.name = entry.obj->GetName();
            if (info.name.empty()) {
                info.name = "Ингредиент";
            }
            info.baseNeed = static_cast<std::uint32_t>(entry.count);
            info.need = info.baseNeed * recipe.multiplier;
            info.have = 0;
            info.missing = info.need;
            recipe.ingredients.push_back(info);
        }
        return RE::BSContainer::ForEachResult::kContinue;
    });

    if (recipe.ingredients.empty()) {
        Dbg("Recipe has no ingredients: " + recipe.itemName);
        return false;
    }

    m_recipes.push_back(std::move(recipe));
    RebuildNeededIngredients();
    RefreshInventory(false);

    Dbg("Tracked recipe: " + m_recipes.back().itemName +
        " (mult=" + std::to_string(recipe.multiplier) +
        ", ingredients=" + std::to_string(m_recipes.back().ingredients.size()) + ")");

    NotifyStateChanged();
    return true; // Added
}

std::uint32_t TrackerManager::CycleRecipeMultiplier(RE::BGSConstructibleObject* a_cobj)
{
    if (!a_cobj) return 0;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    const auto formID = a_cobj->GetFormID();

    auto it = std::find_if(m_recipes.begin(), m_recipes.end(),
        [formID](const TrackedRecipe& r) { return r.cobjFormID == formID; });

    if (it == m_recipes.end()) {
        AddRecipe(a_cobj, 2);
        return 2;
    }

    // Progression: 1 -> 2 -> 3 -> 5 -> 10 -> 25 -> 50 -> 1
    std::uint32_t next = 1;
    switch (it->multiplier) {
    case 1:  next = 2;  break;
    case 2:  next = 3;  break;
    case 3:  next = 5;  break;
    case 5:  next = 10; break;
    case 10: next = 25; break;
    case 25: next = 50; break;
    default: next = 1;  break;
    }

    it->multiplier = next;
    for (auto& ing : it->ingredients) {
        ing.need = ing.baseNeed * it->multiplier;
        ing.missing = (ing.have >= ing.need) ? 0 : (ing.need - ing.have);
    }

    RefreshInventory(false);
    NotifyStateChanged();
    Dbg("Cycled multiplier for " + it->itemName + " to x" + std::to_string(next));
    return next;
}

bool TrackerManager::UntrackRecipe(RE::FormID a_cobjFormID)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = std::find_if(m_recipes.begin(), m_recipes.end(),
        [a_cobjFormID](const TrackedRecipe& r) { return r.cobjFormID == a_cobjFormID; });

    if (it != m_recipes.end()) {
        std::string name = it->itemName;
        m_recipes.erase(it);
        RebuildNeededIngredients();
        Dbg("Untracked recipe by ID: " + name + " (0x" + std::to_string(a_cobjFormID) + ")");
        NotifyStateChanged();
        return true;
    }
    return false;
}

bool TrackerManager::OnItemCrafted(RE::FormID a_createdFormID)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = std::find_if(m_recipes.begin(), m_recipes.end(),
        [a_createdFormID](const TrackedRecipe& r) {
            return r.createdFormID == a_createdFormID;
        });

    if (it != m_recipes.end()) {
        std::string name = it->itemName;

        if (it->multiplier > 1) {
            it->multiplier--;
            for (auto& ing : it->ingredients) {
                ing.need = ing.baseNeed * it->multiplier;
                ing.missing = (ing.have >= ing.need) ? 0 : (ing.need - ing.have);
            }
            Dbg("Recipe crafted (batch): " + name + ", remaining x" + std::to_string(it->multiplier));

            if (Config::GetSingleton()->showNotifications) {
                std::string msg = "Скрафчено! Осталось: " + std::to_string(it->multiplier) + " шт.";
                RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), "UIMenuOK");
            }

            RefreshInventory(false);
            NotifyStateChanged();
            return true;
        }

        m_recipes.erase(it);
        RebuildNeededIngredients();
        Dbg("Recipe automatically completed & removed on craft: " + name + " (0x" + std::to_string(a_createdFormID) + ")");

        if (Config::GetSingleton()->showNotifications) {
            std::string msg = "Скрафчено: " + name;
            RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), "UIMenuOK");
        }

        NotifyStateChanged();
        return true;
    }
    return false;
}

void TrackerManager::Clear()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_recipes.clear();
    m_neededIngredients.clear();
    Dbg("TrackerManager cleared");
    NotifyStateChanged();
}

bool TrackerManager::IsTracked(RE::FormID a_cobjFormID) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (const auto& r : m_recipes) {
        if (r.cobjFormID == a_cobjFormID) return true;
    }
    return false;
}

bool TrackerManager::NeedsIngredient(RE::FormID a_baseFormID) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_neededIngredients.find(a_baseFormID) != m_neededIngredients.end();
}

void TrackerManager::RebuildNeededIngredients()
{
    m_neededIngredients.clear();
    for (const auto& r : m_recipes) {
        for (const auto& ing : r.ingredients) {
            m_neededIngredients.insert(ing.formID);
        }
    }
}

void TrackerManager::RefreshInventory(bool a_notifyUI)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_recipes.empty()) {
        if (a_notifyUI) NotifyStateChanged();
        return;
    }

    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    auto invCounts = player->GetInventoryCounts();

    bool stateChanged = false;

    for (auto& recipe : m_recipes) {
        bool allIngredientsReady = true;

        for (auto& ing : recipe.ingredients) {
            auto* boundObj = RE::TESForm::LookupByID<RE::TESBoundObject>(ing.formID);
            std::uint32_t currentHave = 0;

            if (boundObj) {
                auto it = invCounts.find(boundObj);
                if (it != invCounts.end() && it->second > 0) {
                    currentHave = static_cast<std::uint32_t>(it->second);
                }
            }

            if (ing.have != currentHave) {
                ing.have = currentHave;
                ing.missing = (ing.have >= ing.need) ? 0 : (ing.need - ing.have);
                stateChanged = true;
            }

            if (ing.missing > 0) {
                allIngredientsReady = false;
            }
        }

        if (recipe.isReady != allIngredientsReady) {
            recipe.isReady = allIngredientsReady;
            stateChanged = true;

            if (recipe.isReady) {
                std::string msg = "Крафт готов: " + recipe.itemName;
                RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), "UIMenuOK");
                Dbg("Recipe READY: " + recipe.itemName);
            }
        }
    }

    if (stateChanged && a_notifyUI) {
        NotifyStateChanged();
    }
}

std::string TrackerManager::GetJsonState() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return JsonHelper::SerializeRecipes(m_recipes);
}

std::size_t TrackerManager::GetTrackedCount() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_recipes.size();
}

std::vector<RE::FormID> TrackerManager::GetTrackedRecipeIDs() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    std::vector<RE::FormID> ids;
    ids.reserve(m_recipes.size());
    for (const auto& r : m_recipes) {
        ids.push_back(r.cobjFormID);
    }
    return ids;
}

void TrackerManager::NotifyStateChanged() const
{
    if (m_onStateChanged) {
        std::string json = GetJsonState();
        m_onStateChanged(json);
    }
}
