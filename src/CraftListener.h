#pragma once

#include "pch.h"
#include <RE/I/ItemCrafted.h>
#include <RE/B/BSTEvent.h>

class CraftListener : public RE::BSTEventSink<RE::ItemCrafted::Event>
{
public:
    static CraftListener* GetSingleton()
    {
        static CraftListener singleton;
        return &singleton;
    }

    void Register();
    void Unregister();

    RE::BSEventNotifyControl ProcessEvent(
        const RE::ItemCrafted::Event* a_event,
        RE::BSTEventSource<RE::ItemCrafted::Event>* a_eventSource) override;

private:
    CraftListener() = default;
    ~CraftListener() = default;
    CraftListener(const CraftListener&) = delete;
    CraftListener& operator=(const CraftListener&) = delete;

    std::atomic<bool> m_registered{ false };
};
