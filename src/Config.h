#pragma once

#include "pch.h"
#include <string>

class Config
{
public:
    static Config* GetSingleton()
    {
        static Config singleton;
        return &singleton;
    }

    void Load();
    void Save();

    std::uint32_t hotkey{ 0x21 };          // 'F' key (DirectX scan code 33)
    std::uint32_t toggleHudHotkey{ 0x23 }; // 'H' key (DirectX scan code 35)
    bool          showNotifications{ true };
    bool          playSounds{ true };
    bool          hideWhenHudHidden{ true };

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string GetConfigPath() const;
};
