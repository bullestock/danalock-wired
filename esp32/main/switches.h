#pragma once

#include <atomic>

class Switches
{
public:
    /// Configure switch GPIO pins
    Switches();

    /// Must be called often in order to not miss any state changes
    void update();
    
    bool is_door_closed() const;

    bool is_handle_raised() const;

private:
    std::atomic<bool> m_handle_raised{false};
};

extern Switches switches;
