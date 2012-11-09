/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */


#include <stdio.h>
#include <stdlib.h>
#include "cal.h"
#include "appt.h"


struct cal * cal_init ()
{
    struct cal *cal = malloc(sizeof(struct cal));

    if (cal==NULL)
        return NULL;

    cal->appts = vector_init();

    if (cal->appts == NULL) {
        free(cal);
        return NULL;
    }

    return cal;
}


void cal_destroy (struct cal *cal)
{
    while (cal->appts->elements != 0) {
        appt_destroy(vector_remove(cal->appts, cal->appts->elements -1));
    }

    vector_destroy(cal->appts);
    free(cal);
}


void cal_dump (const struct cal *cal)
{
    struct vector *v = cal->appts;

    if (v!=NULL) {
        for(unsigned int i = 0; i<v->elements; ++i) {
            appt_dump(vector_get(v, i));
            printf("\n");
        }
    }

}


bool cal_save (const struct cal *cal, const char *filename)
{
    FILE *file = fopen(filename, "w");

    if (file==NULL)
        return false;

    for (unsigned int i=0; i < cal->appts->elements; ++i)
        appt_save(file, (struct appt *) vector_get(cal->appts, i));

    fclose(file);
    return true;

}

