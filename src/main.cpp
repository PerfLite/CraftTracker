#include "pch.h"
#include "DebugLog.h"
#include "Config.h"
#include "TrackerManager.h"
#include "CraftingHook.h"
#include "InventoryListener.h"
#include "InputListener.h"
#include "CraftListener.h"
#include "PrismaBridge.h"
#include "Serialization.h"

namespace
{
    void Initialize()
    {
        Dbg("--- Initializing CraftTracker ---");
        Config::GetSingleton()->Load();
        PrismaBridge::GetSingleton()->Setup();
        InputListener::GetSingleton()->Register();
        InventoryListener::GetSingleton()->Register();
        CraftListener::GetSingleton()->Register();
        Dbg("CraftTracker initialized successfully");
    }

    void Teardown()
    {
        Dbg("--- Shutting down CraftTracker ---");
        PrismaBridge::GetSingleton()->Teardown();
        InputListener::GetSingleton()->Unregister();
        InventoryListener::GetSingleton()->Unregister();
        CraftListener::GetSingleton()->Unregister();
        Dbg("CraftTracker teardown complete");
    }

    void SKSEMessageHandler(SKSE::MessagingInterface::Message* a_msg)
    {
        if (!a_msg) return;

        Dbg("SKSE Message received: type=" + std::to_string(a_msg->type));

        switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            Initialize();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            Dbg("kPostLoadGame: resetting visibility and refreshing inventory");
            SKSE::GetTaskInterface()->AddTask([]() {
                PrismaBridge::GetSingleton()->ResetVisibility();
                TrackerManager::GetSingleton()->RefreshInventory(true);
            });
            break;
        case SKSE::MessagingInterface::kNewGame:
            Dbg("kNewGame: clearing tracker and resetting visibility");
            TrackerManager::GetSingleton()->Clear();
            PrismaBridge::GetSingleton()->ResetVisibility();
            break;
        default:
            // Non-gameplay message or shutdown signal (message->type > 8)
            if (a_msg->type > 8) {
                Teardown();
            }
            break;
        }
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    Dbg("========================================");
    Dbg("CraftTracker SKSEPlugin_Load entered");
    Dbg("========================================");

    auto* messaging = static_cast<SKSE::MessagingInterface*>(
        a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));

    if (!messaging) {
        Dbg("CRITICAL: Failed to acquire SKSE Messaging Interface");
        return false;
    }

    SKSE::Init(a_skse);
    messaging->RegisterListener("SKSE", SKSEMessageHandler);

    auto* serialization = static_cast<SKSE::SerializationInterface*>(
        a_skse->QueryInterface(SKSE::LoadInterface::kSerialization));

    if (serialization) {
        serialization->SetUniqueID(Serialization::kDataKey);
        serialization->SetSaveCallback(Serialization::SaveCallback);
        serialization->SetLoadCallback(Serialization::LoadCallback);
        serialization->SetRevertCallback(Serialization::RevertCallback);
        Dbg("Serialization registered with key 'CRFT'");
    } else {
        Dbg("WARNING: Serialization interface null");
    }

    Dbg("CraftTracker load finished");
    return true;
}
