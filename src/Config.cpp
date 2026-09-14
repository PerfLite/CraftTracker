#include "pch.h"
#include "Config.h"
#include "DebugLog.h"
#include <filesystem>
#include <sstream>

std::string Config::GetConfigPath() const
{
    return "Data/SKSE/Plugins/CraftTracker.ini";
}

void Config::Load()
{
    const auto path = GetConfigPath();
    std::ifstream file(path);
    if (!file.is_open()) {
        Dbg("Config not found at " + path + "; generating default config");
        Save();
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Strip comments
        auto commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        // Trim whitespace
        auto trim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        };
        trim(key);
        trim(val);

        if (key == "iHotkey") {
            try {
                hotkey = static_cast<std::uint32_t>(std::stoul(val, nullptr, 0));
            } catch (...) {}
        } else if (key == "iToggleHudHotkey") {
            try {
                toggleHudHotkey = static_cast<std::uint32_t>(std::stoul(val, nullptr, 0));
            } catch (...) {}
        } else if (key == "bShowNotifications") {
            showNotifications = (val == "1" || val == "true" || val == "True");
        } else if (key == "bPlaySounds") {
            playSounds = (val == "1" || val == "true" || val == "True");
        } else if (key == "bHideWhenHudHidden") {
            hideWhenHudHidden = (val == "1" || val == "true" || val == "True");
        }
    }

    Dbg("Config loaded: hotkey=0x" + std::to_string(hotkey) +
        " toggleHudHotkey=0x" + std::to_string(toggleHudHotkey) +
        " showNotif=" + (showNotifications ? "1" : "0"));
}

void Config::Save()
{
    const auto path = GetConfigPath();
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        std::ofstream file(path);
        if (file.is_open()) {
            file << "[General]\n";
            file << "; DirectX scan code for Pin/Unpin hotkey in crafting menu (0x21 / 33 = F, 0x14 / 20 = T, 0x25 / 37 = K)\n";
            file << "iHotkey = " << hotkey << "\n\n";
            file << "; DirectX scan code for Toggle HUD on/off in gameplay (0x23 / 35 = H, 0 = disabled)\n";
            file << "iToggleHudHotkey = " << toggleHudHotkey << "\n\n";
            file << "; Show HUD notification when recipe is pinned, unpinned, or ready\n";
            file << "bShowNotifications = " << (showNotifications ? "true" : "false") << "\n\n";
            file << "; Play sound on actions\n";
            file << "bPlaySounds = " << (playSounds ? "true" : "false") << "\n\n";
            file << "; Auto hide widget when game menus/cutscenes hide HUD\n";
            file << "bHideWhenHudHidden = " << (hideWhenHudHidden ? "true" : "false") << "\n";
            Dbg("Default config written to " + path);
        }
    } catch (...) {
        Dbg("Failed to write config to " + path);
    }
}
