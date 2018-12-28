#pragma once

#include <Windows.h>

class PerformanceTimer
{
private:
    __int64 _start_tick;

public:
    PerformanceTimer(bool autostart = true)
    {
        if (autostart)
        {
            start();
        }
    }

    inline __int64 start()
    {
        LARGE_INTEGER start_tick;

        if (!QueryPerformanceCounter(&start_tick))
        {
            start_tick.QuadPart = 0;
        }

        _start_tick = start_tick.QuadPart;

        return _start_tick;
    }

    inline double elapsed_seconds(__int64 from_specified_tick = -1)
    {
        LARGE_INTEGER current_tick;
        if (!QueryPerformanceCounter(&current_tick))
        {
            current_tick.QuadPart = 0;
        }

        LARGE_INTEGER frequency;
        if (!QueryPerformanceFrequency(&frequency))
        {
            frequency.QuadPart = 1;
        }

        return static_cast<double>(current_tick.QuadPart - (from_specified_tick > 0 ? from_specified_tick : _start_tick)) / frequency.QuadPart;
    }

    inline double reset()
    {
        auto timing = elapsed_seconds();

        start();

        return timing;
    }

    inline static __int64 get_current_tick()
    {
        LARGE_INTEGER current_tick;

        if (!QueryPerformanceCounter(&current_tick))
        {
            current_tick.QuadPart = 0;
        }

        return current_tick.QuadPart;
    }

    inline static double elapsed_seconds(__int64 from_tick, __int64 to_tick)
    {
        LARGE_INTEGER frequency;
        if (!QueryPerformanceFrequency(&frequency))
        {
            frequency.QuadPart = 1;
        }

        return static_cast<double>(to_tick - from_tick) / frequency.QuadPart;
    }
};