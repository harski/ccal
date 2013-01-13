/* Copyright (C) 2013 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include <stdlib.h>
#include "timeutils.h"


struct tm *get_today(struct tm *tm)
{
    time_t tmp_time = time(NULL);

    if (tm==NULL) {
        tm =  malloc(sizeof(struct tm));
        if (tm==NULL)
            return NULL;
    }

    return localtime_r(&tmp_time, tm);
}


void next_day (struct tm *tm)
{
    time_t now = mktime(tm);
    now += 60*60*24;
    localtime_r(&now, tm);
}


void prev_day (struct tm *tm)
{
    time_t now = mktime(tm);
    now -= 60*60*24;
    localtime_r(&now, tm);
}


bool same_day (const struct tm *tm1, const struct tm *tm2)
{
    if (tm1->tm_year == tm2->tm_year &&
        tm1->tm_mon == tm2->tm_mon &&
        tm1->tm_mday == tm2->tm_mday)
        return true;

    return false;
}

