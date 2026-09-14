#include "pch.h"
#include "InputListener.h"
#include "CraftingHook.h"
#include "TrackerManager.h"
#include "PrismaBridge.h"
#include "Config.h"
#include "DebugLog.h"
#include <RE/B/BSInputDeviceManager.h>
#include <RE/B/ButtonEvent.h>
#include <RE/S/SendHUDMessage.h>

void InputListener::Register()
{
    if (m_registered.exchange(true)) return;

    auto* inputDevice = RE::BSInputDeviceManager::GetSingleton();
    if (inputDevice) {
        inputDevice->AddEventSink(this);
        Dbg("InputListener registered for BSInputDeviceManager");
    } else {
        Dbg("BSInputDeviceManager null; cannot register InputListener");
    }
}

void InputListener::Unregister()
{
    if (!m_registered.exchange(false)) return;

    auto* inputDevice = RE::BSInputDeviceManager::GetSingleton();
    if (inputDevice) {
        inputDevice->RemoveEventSink(this);
        Dbg("InputListener unregistered");
    }
}

RE::BSEventNotifyControl InputListener::ProcessEvent(
    RE::InputEvent* const* a_event,
    RE::BSTEventSource<RE::InputEvent*>*)
{
    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    const auto targetHotkey = Config::GetSingleton()->hotkey;
    const auto toggleHudHotkey = Config::GetSingleton()->toggleHudHotkey;

    for (auto* event = *a_event; event; event = event->next) {
        auto* button = event->AsButtonEvent();
        if (!button) continue;

        const auto keyCode = button->GetIDCode();

        // Track Shift key state (0x2A = Left Shift, 0x36 = Right Shift)
        if (keyCode == 0x2A || keyCode == 0x36) {
            m_shiftDown.store(button->IsPressed());
        }

        if (button->IsDown()) {
            auto now = std::chrono::steady_clock::now();

            // 1. Crafting Menu Hotkey (Pin/Unpin or Cycle Multiplier with Shift)
            if (keyCode == targetHotkey) {
                static std::chrono::steady_clock::time_point lastCraftKeyTime{};
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCraftKeyTime).count();
                if (elapsed > 300) {
                    lastCraftKeyTime = now;
                    if (CraftingHook::IsCraftingMenuOpen()) {
                        bool shiftPressed = m_shiftDown.load();
                        Dbg("Hotkey pressed in CraftingMenu (shift=" + std::to_string(shiftPressed) + ")");

                        auto* cobj = CraftingHook::GetCurrentCraftingRecipe();
                        if (cobj) {
                            SKSE::GetTaskInterface()->AddTask([cobj, shiftPressed]() {
                                std::string name;
                                if (cobj->createdItem) {
                                    name = cobj->createdItem->GetName();
                                }
                                if (name.empty()) {
                                    name = cobj->GetName();
                                }
                                if (name.empty()) {
                                    name = "Рецепт";
                                }

                                if (shiftPressed) {
                                    // Shift + F: cycle batch multiplier (x1, x2, x3, x5, x10, x25, x50)
                                    uint32_t mult = TrackerManager::GetSingleton()->CycleRecipeMultiplier(cobj);
                                    if (Config::GetSingleton()->showNotifications) {
                                        std::string msg = "Количество: x" + std::to_string(mult) + " (" + name + ")";
                                        RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), "UIMenuFocus");
                                    }
                                } else {
                                    // Normal F: Toggle Pin / Unpin
                                    bool added = TrackerManager::GetSingleton()->ToggleRecipe(cobj);
                                    if (Config::GetSingleton()->showNotifications) {
                                        std::string msg = added ? ("Крафт-трекер: Закреплено " + name) :
                                                                  ("Крафт-трекер: Откреплено " + name);
                                        const char* sound = Config::GetSingleton()->playSounds ?
                                            (added ? "UIMenuOK" : "UIMenuCancel") : nullptr;
                                        RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), sound);
                                    }
                                }
                            });
                        } else {
                            Dbg("No recipe selected under cursor");
                        }
                    }
                }
            }
            // 2. Toggle HUD visibility hotkey in gameplay (default 'H', keycode 35)
            else if (toggleHudHotkey != 0 && keyCode == toggleHudHotkey) {
                static std::chrono::steady_clock::time_point lastToggleTime{};
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastToggleTime).count();
                if (elapsed > 400) {
                    lastToggleTime = now;
                    if (!CraftingHook::IsCraftingMenuOpen()) {
                        Dbg("Toggle HUD hotkey pressed (keyCode=" + std::to_string(keyCode) + ")");
                        SKSE::GetTaskInterface()->AddTask([]() {
                            PrismaBridge::GetSingleton()->ToggleUserVisibility();
                        });
                    }
                }
            }
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}
