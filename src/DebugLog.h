#pragma once

#include <SKSE/SKSE.h>
#include <string_view>

inline void Dbg(std::string_view a_msg)
{
    SKSE::log::info("{}", a_msg);
}
