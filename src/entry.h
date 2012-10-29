/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef ENTRY_H
#define ENTRY_H

#include <stdbool.h>
#include <time.h>
#include <vector.h>
#include <stdio.h>

struct entry {
    char *header;
    char *description;
    char *category;
    struct tm start;
    struct tm end;
};


struct entry *entry_init();
void entry_destroy (struct entry *entry);

bool entry_add_interactive (struct vector *entries);
void entry_dump (struct entry *entry);
bool entry_save (FILE *file, struct entry *entry);
bool entry_validate (const struct *entry);

#endif /* ENTRY_H */

