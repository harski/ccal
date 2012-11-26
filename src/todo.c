/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE

#include "log.h"
#include "strutils.h"
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


int todo_parse_properties (struct todo *todo, char *key, char *value)
{
    int retval = 1;

    /* check if content */
    if(!strcmp("header", key)) {
        todo->header = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(todo->header, value);
    } else if (!strcmp("description", key)) {
        todo->description = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(todo->description, value);
    } else if (!strcmp("category", key)) {
        todo->category = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(todo->category, value);
    } else if (!strcmp("deadline", key)) {
        if (is_numeric(value)) {
            time_t t = (time_t) atoi(value);
            todo->deadline = malloc(sizeof(struct tm));
            localtime_r(&t, todo->deadline);
        } else {
            /* report error */
            do_log(LL_ERROR, "'%s' is not numeric!",
                   value);
            retval = 0;
        }
    } else if (!strcmp("scheduled", key)) {
        char start[32];
        char end[32];
        str_to_key_value_pairs(value, ':', start, 32, end, 32);

        if (start[0]=='\0' || end[0]=='\0') {
            retval=0;
        } else if (is_numeric(start) && is_numeric(end)) {
                struct timeframe *tf = timeframe_init_alloc();
                time_t tmp = (time_t) atoi(start);
                localtime_r(&tmp, tf->start);
                tmp = (time_t) atoi(end);
                localtime_r(&tmp, tf->end);
        } else {
            /* Value is empty or not numeric */
            do_log(LL_ERROR, "Value of key '%s' is not numeric",
                   key);
            retval = 0;
        }
    } else {
        do_log(LL_WARNING, "Parse error in calfile: key '%s' isn't a property!", key);
        retval = 0;
    }

    return retval;
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
        fprintf(file, "scheduled=\"%d:%d\"\n", (int)mktime(tf->start), (int)mktime(tf->end));
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

