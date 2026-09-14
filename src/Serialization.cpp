#include "pch.h"
#include "Serialization.h"
#include "TrackerManager.h"
#include "PrismaBridge.h"
#include "DebugLog.h"

namespace Serialization
{
    void SaveCallback(SKSE::SerializationInterface* a_intfc)
    {
        if (!a_intfc) return;

        if (!a_intfc->OpenRecord(kDataKey, kSerializationVersion)) {
            Dbg("Serialization: Failed to open record for saving");
            return;
        }

        const auto ids = TrackerManager::GetSingleton()->GetTrackedRecipeIDs();
        std::uint32_t count = static_cast<std::uint32_t>(ids.size());

        a_intfc->WriteRecordData(&count, sizeof(count));
        for (const auto& id : ids) {
            a_intfc->WriteRecordData(&id, sizeof(id));
        }

        Dbg("Serialization: Saved " + std::to_string(count) + " tracked recipes to cosave");
    }

    void LoadCallback(SKSE::SerializationInterface* a_intfc)
    {
        if (!a_intfc) return;

        std::uint32_t type = 0;
        std::uint32_t version = 0;
        std::uint32_t length = 0;

        while (a_intfc->GetNextRecordInfo(type, version, length)) {
            if (type == kDataKey) {
                if (version != kSerializationVersion) {
                    Dbg("Serialization: Unsupported version " + std::to_string(version));
                    continue;
                }

                std::uint32_t count = 0;
                if (!a_intfc->ReadRecordData(&count, sizeof(count))) {
                    Dbg("Serialization: Failed to read recipe count");
                    break;
                }

                Dbg("Serialization: Loading " + std::to_string(count) + " tracked recipes from cosave");

                for (std::uint32_t i = 0; i < count; ++i) {
                    RE::FormID oldID = 0;
                    if (!a_intfc->ReadRecordData(&oldID, sizeof(oldID))) {
                        Dbg("Serialization: Failed to read recipe ID at index " + std::to_string(i));
                        break;
                    }

                    RE::FormID newID = 0;
                    if (a_intfc->ResolveFormID(oldID, newID)) {
                        auto* cobj = RE::TESForm::LookupByID<RE::BGSConstructibleObject>(newID);
                        if (cobj) {
                            TrackerManager::GetSingleton()->AddRecipe(cobj);
                            Dbg("Serialization: Restored recipe FormID=0x" + std::to_string(newID));
                        } else {
                            Dbg("Serialization: FormID 0x" + std::to_string(newID) + " is not a valid COBJ");
                        }
                    } else {
                        Dbg("Serialization: Could not resolve old FormID=0x" + std::to_string(oldID));
                    }
                }
            }
        }
    }

    void RevertCallback(SKSE::SerializationInterface*)
    {
        Dbg("Serialization: Revert requested, clearing tracked recipes and resetting visibility");
        TrackerManager::GetSingleton()->Clear();
        PrismaBridge::GetSingleton()->ResetVisibility();
    }
}
