/* Copyright (C) 2013 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <stdbool.h>
#include <time.h>


/* If tm is NULL it is allocated. Returns NULL on error */
struct tm *get_today(struct tm *tm);

/* Advance tm 24 hours */
void next_day (struct tm *tm);

/* Decrease tm 24 hours */
void prev_day (struct tm *tm);

/* returns true if tm1 and tm2 are the same day
 * (NOTE! Not the same time) */
bool same_day (const struct tm *tm1, const struct tm *tm2);

#endif /* TIMEUTILS_H */

