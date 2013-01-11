/* Copyright (C) 2012-2013 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef TODO_H
#define TODO_H

#include "timeframe.h"
#include "vector.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>


enum Status {
    TS_TODO,
    TS_SCHEDULED,
    TS_WAITING,
    TS_CANCELLED,
    TS_DONE,
    TS_COUNT
};

enum TodoField {
    TODO_HEADER,
    TODO_DESCRIPTION,
    TODO_CATEGORY,
    TODO_STATUS,
    TODO_DEADLINE,
    TODO_SCHEDULED
};

enum todo_type {
    ENTRY_TODO_DEADLINE,
    ENTRY_TODO_SCHEDULED
};

struct todo {
    char *header;
    char *description;
    char *category;
    enum Status status;
    struct vector *scheduled; /* struct timeframes be here */
    struct tm *deadline;
};

struct todo_entry {
    void *todo;
    enum todo_type type;

    union {
        const struct timeframe *scheduled;
        const struct tm *deadline;
    };
};


struct todo *todo_init();
void todo_destroy (struct todo *todo);
const char * todo_get_status_name (enum Status s);
int todo_parse_properties (struct todo *todo, char *key, char *value);
bool todo_save (FILE *file, struct todo *todo);
bool todo_validate (struct todo *todo);

struct todo_entry *todo_entry_init_deadline (const struct todo *todo);
struct todo_entry *todo_entry_init_scheduled (const struct todo *todo,
                                              const struct timeframe *tf);
void todo_entry_destroy (struct todo_entry *todo);

#endif /* TODO_H */

