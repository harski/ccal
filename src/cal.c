/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */


#include <stdio.h>
#include <stdlib.h>
#include "cal.h"
#include "entry.h"


struct cal * cal_init ()
{
    struct cal *cal = malloc(sizeof(struct cal));

    if (cal==NULL)
        return NULL;

    cal->entries = vector_init();

    if (cal->entries == NULL) {
        free(cal);
        return NULL;
    }

    return cal;
}


void cal_destroy (struct cal *cal)
{
    while (cal->entries->elements != 0) {
        entry_destroy(vector_remove(cal->entries, cal->entries->elements -1));
    }

    vector_destroy(cal->entries);
    free(cal);
}


void cal_dump (const struct cal *cal)
{
    struct vector *v = cal->entries;

    if (v!=NULL) {
        for(unsigned int i = 0; i<v->elements; ++i) {
            entry_dump(vector_get(v, i));
            printf("\n");
        }
    }

}


bool cal_save (const struct cal *cal, const char *filename)
{
    FILE *file = fopen(filename, "w");

    if (file==NULL)
        return false;

    for (unsigned int i=0; i < cal->entries->elements; ++i)
        entry_save(file, (struct entry *) vector_get(cal->entries, i));

    fclose(file);
    return true;

}

