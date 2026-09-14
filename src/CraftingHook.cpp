#include "pch.h"
#include "CraftingHook.h"
#include "DebugLog.h"

#include <RE/U/UI.h>
#include <RE/C/CraftingMenu.h>
#include <RE/C/ConstructibleObjectMenu.h>
#include <RE/S/SmithingMenu.h>

namespace CraftingHook
{
    bool IsCraftingMenuOpen()
    {
        auto* ui = RE::UI::GetSingleton();
        return ui && ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME);
    }

    RE::BGSConstructibleObject* GetCurrentCraftingRecipe()
    {
        auto* ui = RE::UI::GetSingleton();
        if (!ui || !ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME)) {
            return nullptr;
        }

        auto craftingMenu = ui->GetMenu<RE::CraftingMenu>(RE::CraftingMenu::MENU_NAME);
        if (!craftingMenu) {
            Dbg("CraftingMenu is open but GetMenu returned null");
            return nullptr;
        }

        auto* subMenu = craftingMenu->GetCraftingSubMenu();
        if (!subMenu) {
            Dbg("CraftingMenu subMenu is null");
            return nullptr;
        }

        // 1. ConstructibleObjectMenu: Forge, Smelter, Tanning Rack, Cooking Pot
        if (auto* cMenu = skyrim_cast<RE::CraftingSubMenus::ConstructibleObjectMenu*>(subMenu)) {
            std::uint32_t idx = cMenu->currentIndex;
            if (idx < cMenu->recipes.size()) {
                auto* cobj = cMenu->recipes[idx].constructibleObject;
                if (cobj) {
                    Dbg("ConstructibleObjectMenu: selected recipe idx=" + std::to_string(idx) +
                        " FormID=0x" + std::to_string(cobj->GetFormID()));
                    return cobj;
                }
            } else {
                Dbg("ConstructibleObjectMenu: currentIndex " + std::to_string(idx) +
                    " out of range (size=" + std::to_string(cMenu->recipes.size()) + ")");
            }
        }
        // 2. SmithingMenu: Armor/Weapon Workbench & Grindstone
        else if (auto* sMenu = skyrim_cast<RE::CraftingSubMenus::SmithingMenu*>(subMenu)) {
            std::uint32_t idx = sMenu->currentIndex;
            if (idx < sMenu->recipes.size()) {
                auto* cobj = sMenu->recipes[idx].constructibleObject;
                if (cobj) {
                    Dbg("SmithingMenu: selected recipe idx=" + std::to_string(idx) +
                        " FormID=0x" + std::to_string(cobj->GetFormID()));
                    return cobj;
                }
            } else {
                Dbg("SmithingMenu: currentIndex " + std::to_string(idx) +
                    " out of range (size=" + std::to_string(sMenu->recipes.size()) + ")");
            }
        } else {
            Dbg("Other CraftingSubMenu (Alchemy/Enchanting)");
        }

        return nullptr;
    }
}
