/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "appt.h"
#include "log.h"
#include "strutils.h"


#define INPUT_BUFFER_SIZE 128


struct appt *appt_init()
{
    struct appt *appt = malloc(sizeof(struct appt));
    struct timeframe *tf = timeframe_init();

    if (appt==NULL || tf==NULL)
        goto init_error;

    appt->tf = tf;

    appt->header = NULL;
    appt->description = NULL;
    appt->category = NULL;

    return appt;

init_error:
    free(appt);
    timeframe_destroy(tf);
    return NULL;
}


int appt_parse_properties (struct appt *appt, char *key, char *value)
{
    int retval = 1;

    /* check if content */
    if(!strcmp("header", key)) {
        appt->header = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(appt->header, value);
    } else if (!strcmp("description", key)) {
        appt->description = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(appt->description, value);
    } else if (!strcmp("category", key)) {
        appt->category = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(appt->category, value);
    } else if (!strcmp("start", key)) {
        if (appt->tf->start == NULL) {
            appt->tf->start = malloc(sizeof(struct tm));
        } else {
            /* the save entry is broken, having multiple start times */
            do_log(LL_WARNING, "Error: appt save entry is broken, appt has"
                               " multiple start times. (here key='%s' value='%s'", key, value);
        }

        if (is_numeric(value)) {
            time_t t = (time_t) atoi(value);
            localtime_r(&t, appt->tf->start);
        } else {
            do_log(LL_WARNING, "%s", "Appt start time is not numeric");
            retval = 0;
        }
    } else if (!strcmp("end", key)) {
        if (appt->tf->end == NULL) {
            appt->tf->end = malloc(sizeof(struct tm));
        } else {
            /* the save entry is broken, having multiple end times */
            do_log(LL_WARNING, "Error: appt save entry is broken, appt has"
                               " multiple end times. (here key='%s' value='%s'", key, value);
        }

        if (is_numeric(value)) {
            time_t t = (time_t) atoi(value);
            localtime_r(&t, appt->tf->end);
        } else {
            do_log(LL_WARNING, "%s", "Appt end time is not numeric");
            retval = 0;
        }
    } else {
        do_log(LL_WARNING, "Error parsing calfile: key '%s' isn't a property!\n", key);
        retval = 0;
    }

    return retval;
}


bool appt_save (FILE *file, struct appt *appt)
{
    fprintf(file, "APPT-START\n");
    fprintf(file, "header=\"%s\"\n", appt->header);

    if (appt->description!=NULL)
        fprintf(file, "description=\"%s\"\n", appt->description);

    if (appt->category!=NULL)
        fprintf(file, "category=\"%s\"\n", appt->category);

    fprintf(file, "start=%d\n", (int)mktime(appt->tf->start));
    fprintf(file, "end=%d\n", (int)mktime(appt->tf->end));

    fprintf(file, "APPT-END\n\n");

    return true;
}


void appt_destroy (struct appt *appt)
{
    free(appt->header);
    free(appt->description);
    free(appt->category);
    if (appt->tf!=NULL) {
        free(appt->tf->start);
        free(appt->tf->end);
        free(appt->tf);
    }
    free(appt);
}


void appt_dump (struct appt *appt)
{
    size_t size = 32;
    char start[size];
    char end[size];

    strftime(start, size, "%F %H:%M", appt->tf->start);
    strftime(end, size, "%F %H:%M", appt->tf->end);

    printf ("%s -> %s\n", start, end);
    printf("%s\n", appt->header);

    if (appt->description!=NULL)
        printf("%s\n", appt->description);

    if (appt->category!=NULL)
        printf("Category: %s\n", appt->category);
}


/* Not const, because mktime edits the struct tm */
bool appt_validate (struct appt *appt)
{
    bool valid = true;

    if (appt->header==NULL || appt->header[0] == '\0')
        valid = false;

    if (appt->tf != NULL) {
        if (appt->tf->start != NULL && appt->tf->end != NULL) {
            time_t start, end;

            start = mktime(appt->tf->start);
            end = mktime(appt->tf->end);

            if (end<start)
                valid = false;
        } else {
            valid = false;
        }
    } else {
        valid = false;
    }

    return valid;
}


/* Merge sort */
static void _sort (struct appt **a, size_t size)
{
    struct appt **left;
    struct appt **right;
    size_t left_size = size/2;
    size_t right_size = size - left_size;
    unsigned int i, l, r;

    if (size <=1)
        return;

    left = malloc(sizeof(struct appt *) * left_size);
    right = malloc(sizeof(struct appt *) * right_size);

    for (i=0; i<left_size; ++i)
        left[i] = a[i];

    for (i=0; i<right_size; ++i)
        right[i] = a[left_size+i];

    _sort(left, left_size);
    _sort(right, right_size);

    i = l = r = 0;
    while (i<size) {
        if (l==left_size) {
            a[i] = right[r++];
        } else if (r==right_size) {
            a[i] = left[l++];
        } else {
            if (mktime(left[l]->tf->start) <= mktime(right[r]->tf->start))
                a[i] = left[l++];
            else
                a[i] = right[r++];
        }

        ++i;
    }

    free(left);
    free(right);
}


void appts_sort (struct vector *appts)
{
    struct appt **a;
    size_t size = appts->elements;

    if (size != 0) {
        a = malloc(size*sizeof(struct appt *));

        for (unsigned int i=0; i<size; ++i)
            a[i] = vector_remove_last(appts);

        _sort(a, size);

        for (unsigned int i=0; i<size; ++i)
            vector_add(appts, a[i]);
    }
}


