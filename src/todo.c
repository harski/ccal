/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include "todo.h"
#include <stdlib.h>
#include <string.h>


struct todo *todo_init()
{
    struct todo *t = malloc(sizeof(struct todo));
    struct vector *v = vector_init_size(1);

    if (t==NULL)
        goto todo_init_fail;

    if (v==NULL)
        goto todo_init_fail;

    t->header = NULL;
    t->description = NULL;
    t->category = NULL;
    t->status = TS_TODO;

    return t;

todo_init_fail:
    free(t);
    free(v);
    return NULL;
}


void todo_destroy (struct todo *todo)
{
    if (todo->scheduled!=NULL)
        while(todo->scheduled->elements != 0)
            free(vector_remove(todo->scheduled, 0));

    vector_destroy(todo->scheduled);
    free(todo);
}


bool todo_save (FILE *file, struct todo *todo)
{
    fprintf(file, "TODO-START\n");
    fprintf(file, "header=\"%s\"\n", todo->header);

    if (todo->description != NULL)
        fprintf(file, "description=\"%s\"\n", todo->description);

    if (todo->category != NULL)
        fprintf(file, "category=\"%s\"\n", todo->category);

    /* Save the scheduled timeframes */
    for (unsigned int i=0; i<todo->scheduled->elements; ++i) {
        struct timeframe *tf = (struct timeframe *) vector_get(todo->scheduled, i);
        fprintf(file, "scheduled_start=\"%d\"\n", (int)mktime(tf->start));
        fprintf(file, "scheduled_end=\"%d\"\n", (int)mktime(tf->end));
    }

    if (todo->deadline != NULL)
        fprintf(file, "deadline=\"%d\"\n", (int)mktime(todo->deadline));

    fprintf(file, "TODO-END\n\n");

    return true;
}


bool todo_validate (struct todo *todo)
{
    if (todo->header==NULL || strlen(todo->header)==0)
        return false;

    return true;
}

