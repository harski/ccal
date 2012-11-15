/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef TODO_H
#define TODO_H

#include <vector.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include "timeframe.h"


enum Status {
    TS_TODO,
    TS_SCHEDULED,
    TS_WAITING,
    TS_CANCELLED,
    TS_DONE
};


struct todo {
    char *header;
    char *description;
    char *category;
    enum Status status;
    struct vector *scheduled; /* struct timeframes be here */
    struct tm *deadline;
};

struct todo *todo_init();
void todo_destroy (struct todo *todo);
bool todo_save (FILE *file, struct todo *todo);
bool todo_validate (struct todo *todo);

#endif /* TODO_H */

