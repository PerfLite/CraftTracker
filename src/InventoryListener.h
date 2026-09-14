#pragma once

#include "pch.h"
#include <RE/T/TESContainerChangedEvent.h>
#include <RE/B/BSTEvent.h>

class InventoryListener : public RE::BSTEventSink<RE::TESContainerChangedEvent>
{
public:
    static InventoryListener* GetSingleton()
    {
        static InventoryListener singleton;
        return &singleton;
    }

    void Register();
    void Unregister();

    RE::BSEventNotifyControl ProcessEvent(
        const RE::TESContainerChangedEvent* a_event,
        RE::BSTEventSource<RE::TESContainerChangedEvent>* a_eventSource) override;

private:
    InventoryListener() = default;
    ~InventoryListener() = default;
    InventoryListener(const InventoryListener&) = delete;
    InventoryListener& operator=(const InventoryListener&) = delete;

    std::atomic<bool> m_registered{ false };
};
