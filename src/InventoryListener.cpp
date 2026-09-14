#include "pch.h"
#include "InventoryListener.h"
#include "TrackerManager.h"
#include "DebugLog.h"
#include <RE/S/ScriptEventSourceHolder.h>

void InventoryListener::Register()
{
    if (m_registered.exchange(true)) return;

    auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
    if (holder) {
        holder->AddEventSink<RE::TESContainerChangedEvent>(this);
        Dbg("InventoryListener registered for TESContainerChangedEvent");
    } else {
        Dbg("ScriptEventSourceHolder null; cannot register InventoryListener");
    }
}

void InventoryListener::Unregister()
{
    if (!m_registered.exchange(false)) return;

    auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
    if (holder) {
        holder->RemoveEventSink<RE::TESContainerChangedEvent>(this);
        Dbg("InventoryListener unregistered");
    }
}

RE::BSEventNotifyControl InventoryListener::ProcessEvent(
    const RE::TESContainerChangedEvent* a_event,
    RE::BSTEventSource<RE::TESContainerChangedEvent>*)
{
    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return RE::BSEventNotifyControl::kContinue;
    }

    const auto playerID = player->GetFormID();
    if (a_event->oldContainer != playerID && a_event->newContainer != playerID) {
        return RE::BSEventNotifyControl::kContinue;
    }

    // Fast check if the item is needed by any tracked recipe
    if (!TrackerManager::GetSingleton()->NeedsIngredient(a_event->baseObj)) {
        return RE::BSEventNotifyControl::kContinue;
    }

    Dbg("Inventory change detected for tracked ingredient FormID=0x" + std::to_string(a_event->baseObj) +
        " (count=" + std::to_string(a_event->itemCount) + ")");

    // Defer inventory refresh to the game task loop to ensure thread safety
    SKSE::GetTaskInterface()->AddTask([]() {
        TrackerManager::GetSingleton()->RefreshInventory(true);
    });

    return RE::BSEventNotifyControl::kContinue;
}
