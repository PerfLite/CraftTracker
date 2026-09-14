#include "pch.h"
#include "PrismaBridge.h"
#include "TrackerManager.h"
#include "Config.h"
#include "DebugLog.h"
#include <RE/U/UI.h>

void PrismaBridge::Setup()
{
    m_prismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(
        PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1));

    Dbg("PrismaBridge Setup: interface=" +
        std::to_string(reinterpret_cast<std::uintptr_t>(static_cast<void*>(m_prismaUI))));

    if (!m_prismaUI) {
        Dbg("PrismaUI interface not available");
        return;
    }

    m_view = m_prismaUI->CreateView("CraftTracker/index.html", [](PrismaView v) {
        Dbg("CraftTracker DOM ready (view handle=" + std::to_string(v) + ")");
        auto* bridge = PrismaBridge::GetSingleton();
        bridge->m_domReady.store(true);

        if (!bridge->m_prismaUI) return;

        // Register callback for untracking recipe from JS (e.g. clicking 'X' on a card)
        bridge->m_prismaUI->RegisterJSListener(v, "CraftTracker_untrack", [](const char* arg) {
            if (!arg) return;
            try {
                RE::FormID formID = static_cast<RE::FormID>(std::stoul(arg, nullptr, 0));
                Dbg("JS requested untrack FormID=0x" + std::to_string(formID));
                SKSE::GetTaskInterface()->AddTask([formID]() {
                    TrackerManager::GetSingleton()->UntrackRecipe(formID);
                });
            } catch (...) {
                Dbg("Failed to parse formID from JS: " + std::string(arg));
            }
        });

        // Register callback for clearing all tracked recipes from JS
        bridge->m_prismaUI->RegisterJSListener(v, "CraftTracker_clear", [](const char*) {
            Dbg("JS requested clear all");
            SKSE::GetTaskInterface()->AddTask([]() {
                TrackerManager::GetSingleton()->Clear();
            });
        });

        // Register callback when JS has initialized and requests initial state
        bridge->m_prismaUI->RegisterJSListener(v, "CraftTracker_requestState", [](const char*) {
            Dbg("JS requested initial state");
            SKSE::GetTaskInterface()->AddTask([]() {
                auto json = TrackerManager::GetSingleton()->GetJsonState();
                PrismaBridge::GetSingleton()->SendUpdate(json);
            });
        });

        // Send initial state and reset visibility to visible
        SKSE::GetTaskInterface()->AddTask([]() {
            auto* bridge = PrismaBridge::GetSingleton();
            bridge->ResetVisibility();
            auto json = TrackerManager::GetSingleton()->GetJsonState();
            bridge->SendUpdate(json);
        });
    });

    Dbg("CraftTracker View created handle=" + std::to_string(m_view));

    // Connect TrackerManager updates to PrismaBridge
    TrackerManager::GetSingleton()->SetOnStateChanged([](const std::string& json) {
        PrismaBridge::GetSingleton()->SendUpdate(json);
    });

    // Register for MenuOpenCloseEvent to hide HUD in full-screen menus
    auto* ui = RE::UI::GetSingleton();
    if (ui) {
        ui->AddEventSink<RE::MenuOpenCloseEvent>(this);
        Dbg("Registered for MenuOpenCloseEvent");
    }
}

void PrismaBridge::Teardown()
{
    m_quit.store(true);
    if (m_prismaUI && m_view != 0 && m_prismaUI->IsValid(m_view)) {
        Dbg("Destroying Prisma view on teardown");
        m_prismaUI->Destroy(m_view);
    }
    m_view = 0;
    m_domReady.store(false);
}

void PrismaBridge::SendUpdate(const std::string& a_json)
{
    if (m_quit.load() || !m_domReady.load()) return;

    if (m_prismaUI && m_view != 0 && m_prismaUI->IsValid(m_view)) {
        std::string script = "updateTracker(" + a_json + ");";
        m_prismaUI->Invoke(m_view, script.c_str());

        // If not in a hiding menu and not explicitly hidden by user, ensure it is visible
        if (!m_hudHiddenByMenu.load() && !m_userHidden.load()) {
            m_prismaUI->Show(m_view);
        }
    }
}

void PrismaBridge::SetVisible(bool a_visible)
{
    if (m_quit.load()) return;

    if (m_prismaUI && m_view != 0 && m_prismaUI->IsValid(m_view)) {
        bool show = a_visible && !m_userHidden.load();
        Dbg("SetVisible: req=" + std::to_string(a_visible) +
            " menuHidden=" + std::to_string(m_hudHiddenByMenu.load()) +
            " userHidden=" + std::to_string(m_userHidden.load()) +
            " -> " + (show ? "SHOW" : "HIDE"));
        if (show) {
            m_prismaUI->Show(m_view);
        } else {
            m_prismaUI->Hide(m_view);
        }
    }
}

void PrismaBridge::ResetVisibility()
{
    m_userHidden.store(false);
    m_hudHiddenByMenu.store(false);
    Dbg("PrismaBridge: ResetVisibility invoked (userHidden=0, menuHidden=0)");
    SetVisible(true);
}

void PrismaBridge::ToggleUserVisibility()
{
    bool nowHidden = !m_userHidden.load();
    m_userHidden.store(nowHidden);

    Dbg("User toggled HUD visibility: hidden=" + std::to_string(nowHidden));

    if (nowHidden) {
        SetVisible(false);
    } else {
        if (!m_hudHiddenByMenu.load()) {
            SetVisible(true);
        } else {
            Dbg("User toggled HUD to visible, but a menu is open (m_hudHiddenByMenu=1)");
        }
    }

    if (Config::GetSingleton()->showNotifications) {
        std::string msg = nowHidden ? "Крафт-трекер: скрыт" : "Крафт-трекер: показан";
        const char* sound = nowHidden ? "UIMenuCancel" : "UIMenuOK";
        RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), sound);
    }
}

RE::BSEventNotifyControl PrismaBridge::ProcessEvent(
    const RE::MenuOpenCloseEvent* a_event,
    RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
    if (!a_event || m_quit.load() || !Config::GetSingleton()->hideWhenHudHidden) {
        return RE::BSEventNotifyControl::kContinue;
    }

    const auto menuName = a_event->menuName;

    // Menus that should cause the HUD to be hidden
    static const std::unordered_set<std::string_view> hidingMenus = {
        "Loading Menu",
        "MapMenu",
        "Journal Menu",
        "Credits Menu",
        "StatsMenu",
        "Book Menu",
        "Dialogue Menu",
        "BarterMenu",
        "Crafting Menu",
        "InventoryMenu",
        "ContainerMenu",
        "MagicMenu",
        "GiftMenu",
        "FavoritesMenu",
        "TweenMenu",
        "Console",
        "Lockpicking Menu",
        "Sleep/Wait Menu",
        "LevelUp Menu"
    };

    if (hidingMenus.find(menuName.data()) != hidingMenus.end()) {
        auto* ui = RE::UI::GetSingleton();
        bool anyOpen = false;
        std::string openMenuFound;
        if (ui) {
            if (a_event->opening) {
                anyOpen = true;
                openMenuFound = menuName.data();
            } else {
                for (auto name : hidingMenus) {
                    if (name != menuName.data() && ui->IsMenuOpen(name)) {
                        anyOpen = true;
                        openMenuFound = name;
                        break;
                    }
                }
            }
        }
        m_hudHiddenByMenu.store(anyOpen);
        Dbg(std::string("MenuEvent: ") + menuName.data() + (a_event->opening ? " OPEN" : " CLOSE") +
            " -> anyOpen=" + std::to_string(anyOpen) + (anyOpen ? (" (blocked by " + openMenuFound + ")") : "") +
            " userHidden=" + std::to_string(m_userHidden.load()));
        SetVisible(!anyOpen);
    }

    return RE::BSEventNotifyControl::kContinue;
}
