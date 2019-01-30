#pragma once

#include <atomic>

class spin_lock
{
public:
    void lock()
    {
        while (_flag.test_and_set(std::memory_order_acquire))
        {
        }
    }

    void unlock()
    {
        _flag.clear(std::memory_order_release);
    }
private:
    std::atomic_flag _flag;
};
