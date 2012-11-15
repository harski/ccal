/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef TIMEFRAME_H
#define TIMEFRAME_H

#include <time.h>


struct timeframe {
    struct tm *start;
    struct tm *end;
};


struct timeframe * timeframe_init();
struct timeframe * timeframe_init_alloc();
void timeframe_destroy (struct timeframe *tf);

#endif /* TIMEFRAME_H */

