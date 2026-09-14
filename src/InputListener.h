#pragma once

#include "pch.h"
#include <RE/B/BSTEvent.h>
#include <RE/I/InputEvent.h>

class InputListener : public RE::BSTEventSink<RE::InputEvent*>
{
public:
    static InputListener* GetSingleton()
    {
        static InputListener singleton;
        return &singleton;
    }

    void Register();
    void Unregister();

    RE::BSEventNotifyControl ProcessEvent(
        RE::InputEvent* const* a_event,
        RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;

private:
    InputListener() = default;
    ~InputListener() = default;
    InputListener(const InputListener&) = delete;
    InputListener& operator=(const InputListener&) = delete;

    std::atomic<bool> m_registered{ false };
    std::atomic<bool> m_shiftDown{ false };
};
