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

    /// Called when the door is locked.
    void set_door_locked();

    /// Return true if the door has been open since set_door_locked() was called.
    bool was_door_open() const;

private:
    std::atomic<bool> m_handle_raised{false};
    std::atomic<bool> m_door_locked{false};
};

extern Switches switches;
