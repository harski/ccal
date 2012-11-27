/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef ENTRY_H
#define ENTRY_H

#include "timeframe.h"
#include "vector.h"
#include <stdbool.h>
#include <time.h>
#include <stdio.h>


enum {
    APPT_HEADER,
    APPT_START_TIME,
    APPT_END_TIME,
    APPT_CATEGORY,
    APPT_DESCRIPTION
};

struct appt {
    char *header;
    char *description;
    char *category;
    struct timeframe *tf;
};


struct appt *appt_init();
void appt_destroy (struct appt *appt);

int appt_parse_properties (struct appt *appt, char *key, char *value);
void appt_dump (struct appt *appt);
bool appt_save (FILE *file, struct appt *appt);
bool appt_validate (struct appt *appt);

#endif /* ENTRY_H */

