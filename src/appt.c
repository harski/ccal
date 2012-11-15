/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "appt.h"


#define INPUT_BUFFER_SIZE 128


struct appt *appt_init()
{
    time_t t = time(NULL);
    struct appt *appt = malloc(sizeof(struct appt));
    if (appt==NULL)
        return NULL;

    appt->header = NULL;
    appt->description = NULL;
    appt->category = NULL;

    localtime_r(&t, &appt->start);
    localtime_r(&t, &appt->end);

    return appt;
}


/* TODO: fix unsafe atoi calls */
/* TODO: localtime calls should probably be done with localtime_r */
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
        time_t t = (time_t) atoi(value);
        appt->start = *localtime(&t);
    } else if (!strcmp("end", key)) {
        time_t t = (time_t) atoi(value);
        appt->end = *localtime(&t);
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

    fprintf(file, "start=%d\n", (int)mktime(&appt->start));
    fprintf(file, "end=%d\n", (int)mktime(&appt->end));

    fprintf(file, "APPT-END\n\n");

    return true;
}


void appt_destroy (struct appt *appt)
{
    free(appt->header);
    free(appt->description);
    free(appt->category);
    free(appt);
}


void appt_dump (struct appt *appt)
{
    size_t size = 32;
    char start[size];
    char end[size];

    strftime(start, size, "%F %H:%M", &appt->start);
    strftime(end, size, "%F %H:%M", &appt->end);

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
    start = mktime(&appt->start);
    end = mktime(&appt->end);

    if (appt->header==NULL || end<start)
        return false;

    return true;
}

