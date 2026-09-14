#pragma once

#include "pch.h"
#include "PrismaUI_API.h"
#include <RE/M/MenuOpenCloseEvent.h>
#include <RE/B/BSTEvent.h>

class PrismaBridge : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    static PrismaBridge* GetSingleton()
    {
        static PrismaBridge singleton;
        return &singleton;
    }

    void Setup();
    void Teardown();

    void SendUpdate(const std::string& a_json);
    void SetVisible(bool a_visible);
    void ToggleUserVisibility();
    void ResetVisibility();

    RE::BSEventNotifyControl ProcessEvent(
        const RE::MenuOpenCloseEvent* a_event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;

private:
    PrismaBridge() = default;
    ~PrismaBridge() = default;
    PrismaBridge(const PrismaBridge&) = delete;
    PrismaBridge& operator=(const PrismaBridge&) = delete;

    PRISMA_UI_API::IVPrismaUI1* m_prismaUI{ nullptr };
    PrismaView                  m_view{ 0 };
    std::atomic<bool>           m_quit{ false };
    std::atomic<bool>           m_domReady{ false };
    std::atomic<bool>           m_hudHiddenByMenu{ false };
    std::atomic<bool>           m_userHidden{ false };
};
