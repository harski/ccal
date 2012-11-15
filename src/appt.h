/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef ENTRY_H
#define ENTRY_H

#include <stdbool.h>
#include <time.h>
#include <vector.h>
#include <stdio.h>

struct appt {
    char *header;
    char *description;
    char *category;
    struct tm start;
    struct tm end;
};


struct appt *appt_init();
void appt_destroy (struct appt *appt);

int appt_parse_properties (struct appt *appt, char *key, char *value);
void appt_dump (struct appt *appt);
bool appt_save (FILE *file, struct appt *appt);
bool appt_validate (struct appt *appt);

#endif /* ENTRY_H */

