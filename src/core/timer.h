#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Timer
{
    double target;
    bool _has_timed_out;
    uint64_t _timeouts;
    double _elapsed;
} Timer;

double get_elapsed_seconds(void);
double get_elapsed_milliseconds(void);

void timer_init(Timer *timer, double timeout_seconds);
void timer_update(Timer *timer, double delta);
bool timer_timeout(Timer *timer);
