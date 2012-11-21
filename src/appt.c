/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "appt.h"
#include "strutils.h"


#define INPUT_BUFFER_SIZE 128


struct appt *appt_init()
{
    struct appt *appt = malloc(sizeof(struct appt));
    struct timeframe *tf = malloc(sizeof(struct timeframe));

    if (appt==NULL || tf==NULL)
        goto init_error;

    appt->tf = tf;
    tf->start = NULL;
    tf->end = NULL;

    appt->header = NULL;
    appt->description = NULL;
    appt->category = NULL;

    return appt;

init_error:
    free(appt);
    free(tf);
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
            /* TODO: do something sensible about it. At least report,
             * should we fail, too? */
        }

        if (is_numeric(value)) {
            time_t t = (time_t) atoi(value);
            localtime_r(&t, appt->tf->start);
        } else {
            /* TODO: again, report error */
            retval = 0;
        }
    } else if (!strcmp("end", key)) {
        if (appt->tf->end == NULL) {
            appt->tf->end = malloc(sizeof(struct tm));
        } else {
            /* the save entry is broken, having multiple start times */
            /* TODO: do something sensible about it. At least report,
             * should we fail, too? */
        }

        if (is_numeric(value)) {
            time_t t = (time_t) atoi(value);
            localtime_r(&t, appt->tf->end);
        } else {
            /* TODO: again, report error */
            retval = 0;
        }
    } else {
        fprintf(stderr, "Error parsing calfile: key '%s' isn't a property!\n", key);
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
    time_t start, end;
    start = mktime(appt->tf->start);
    end = mktime(appt->tf->end);

    if (appt->header==NULL || end<start)
        return false;

    return true;
}

