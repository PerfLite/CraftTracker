#pragma once

#include <fstream>
#include <string_view>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

inline void Dbg(std::string_view a_msg)
{
    try {
        std::ofstream f(R"(C:\Users\Alik\Documents\My Games\Skyrim Special Edition\SKSE\CraftTracker_dbg.log)",
                        std::ios::app);
        if (f.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            
            std::tm tm_buf{};
            localtime_s(&tm_buf, &in_time_t);
            f << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
              << a_msg << "\n";
        }
    } catch (...) {
    }
}
