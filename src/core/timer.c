#include <stdint.h>
#include <Windows.h>

#include "timer.h"
#include "utils.h"

double get_elapsed_seconds(void)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);

    return (double)count.QuadPart / (double)freq.QuadPart;    
}

double get_elapsed_milliseconds(void)
{
    return get_elapsed_seconds() * 1000.0;
}

void timer_init(Timer *timer, double timeout_seconds)
{
    clean_struct(timer);
    timer->target = timeout_seconds;
}

void timer_update(Timer *timer, double delta)
{
    timer->_elapsed += delta;
    uint64_t timeouts = (uint64_t)(timer->_elapsed / timer->target);
    timer->_elapsed = timer->_elapsed - timer->target * timeouts;
    timer->_timeouts += timeouts;
    timer->_has_timed_out = false;
}

bool timer_timeout(Timer *timer)
{
    return_if_s(timer->_timeouts == 0, false);
    return_if_s(timer->_has_timed_out, true);

    --timer->_timeouts;
    timer->_has_timed_out = true;
    return true;
}
