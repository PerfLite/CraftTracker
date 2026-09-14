#pragma once

#include "pch.h"

namespace Serialization
{
    constexpr std::uint32_t kSerializationVersion = 1;
    constexpr std::uint32_t kDataKey = 'CRFT';

    void SaveCallback(SKSE::SerializationInterface* a_intfc);
    void LoadCallback(SKSE::SerializationInterface* a_intfc);
    void RevertCallback(SKSE::SerializationInterface* a_intfc);
}
