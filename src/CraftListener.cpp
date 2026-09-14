#include "pch.h"
#include "CraftListener.h"
#include "TrackerManager.h"
#include "DebugLog.h"

void CraftListener::Register()
{
    if (m_registered.exchange(true)) return;

    auto* source = RE::ItemCrafted::GetEventSource();
    if (source) {
        source->AddEventSink(this);
        Dbg("CraftListener registered for ItemCrafted::Event");
    } else {
        Dbg("ItemCrafted::GetEventSource returned null");
    }
}

void CraftListener::Unregister()
{
    if (!m_registered.exchange(false)) return;

    auto* source = RE::ItemCrafted::GetEventSource();
    if (source) {
        source->RemoveEventSink(this);
        Dbg("CraftListener unregistered");
    }
}

RE::BSEventNotifyControl CraftListener::ProcessEvent(
    const RE::ItemCrafted::Event* a_event,
    RE::BSTEventSource<RE::ItemCrafted::Event>*)
{
    if (!a_event || !a_event->item) {
        return RE::BSEventNotifyControl::kContinue;
    }

    const auto craftedFormID = a_event->item->GetFormID();
    Dbg("ItemCrafted::Event received: FormID=0x" + std::to_string(craftedFormID) +
        " Name=" + std::string(a_event->item->GetName()));

    SKSE::GetTaskInterface()->AddTask([craftedFormID]() {
        TrackerManager::GetSingleton()->OnItemCrafted(craftedFormID);
    });

    return RE::BSEventNotifyControl::kContinue;
}
