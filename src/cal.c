/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appt.h"
#include "cal.h"
#include "getline.h"
#include "strutils.h"

#define READ_BUF_SIZE 512


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


int load_cal_file (struct cal *cal, const char *filepath)
{
    FILE *file = fopen(filepath, "r");

    if (file==NULL) {
        fprintf(stderr, "Calendar file '%s' not found\n", filepath);
        return 0;
    }

    size_t buffer_len = READ_BUF_SIZE;
    char *buffer = malloc(sizeof(char)*READ_BUF_SIZE);
    int retval;
    int appt_open = 0;
    unsigned int line = 0;
    struct appt *appt;

    char key[READ_BUF_SIZE];
    char value[READ_BUF_SIZE];

    while (0 < (retval = getline_custom(&buffer, &buffer_len, file))) {
        ++line;

        /* Check if line is empty */
        if (retval <= 1)
            continue;

        strip(buffer, retval);

        /* TODO: check if content */
        if(!strcmp("APPT-START", buffer)) {
            if (appt_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            appt_open = 1;
            appt = appt_init();

        } else if (!strcmp("APPT-END", buffer)) {
            if (!appt_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            /* TODO: validate appt before adding */
            appt_open = 0;
            vector_add(cal->appts, (void *)appt);

        } else if (appt_open &&
                   -1 != str_to_key_value_pairs(buffer, '=', key, READ_BUF_SIZE, value, READ_BUF_SIZE)) {
            strip(key, strlen(key));
            strip(value, strlen(value));
            removequotes(value);

            appt_parse_properties(appt, key, value);
        }

    }

    free(buffer);
    fclose(file);

    return 1;
}

