/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appt.h"
#include "cal.h"
#include "getline.h"
#include "log.h"
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
    int success = 1;
    FILE *file;
    size_t buffer_len = READ_BUF_SIZE;
    char *buffer = malloc(sizeof(char)*READ_BUF_SIZE);
    int retval;
    int appt_open = 0;
    unsigned int line = 0;
    struct appt *appt = NULL;

    char key[READ_BUF_SIZE];
    char value[READ_BUF_SIZE];

    file = fopen(filepath, "r");

    if (file==NULL) {
        do_log(LL_ERROR, "Calendar file '%s' not found or cannot be opened", filepath);
        return 0;
    }

    if (buffer==NULL) {
        do_log(LL_ERROR, "Malloc failed in %s:%s", __FILE__, __func__);
        return 0;
    }

    while (0 < (retval = getline_custom(&buffer, &buffer_len, file)) && success) {
        ++line;

        /* Check if line is empty */
        if (retval <= 1)
            continue;

        strip(buffer, retval);

        /* check if content */
        if(!strcmp("APPT-START", buffer)) {
            if (appt_open) {
                do_log(LL_WARNING, "Syntax error in '%s' near line %u:\n\"%s\"",
                       filepath, line, buffer);
                success = 0;
            } else {
                appt_open = 1;
                appt = appt_init();
            }
        } else if (!strcmp("APPT-END", buffer)) {
            if (!appt_open) {
                /* Check if appt is opened before it's closed */
                do_log(LL_WARNING, "Syntax error in '%s' near line %u:\n\"%s\"",
                       filepath, line, buffer);
                success = 0;
            } else if (appt_validate(appt)) {
                /* validate appt before adding */
                vector_add(cal->appts, (void *)appt);
                appt_open = 0;
                appt = NULL;
            } else {
                /* The parsed appt entry does not validate: dump it */
                do_log(LL_WARNING, "%s", "Invalid appt in savefile");
                success = 0;
            }
        } else if (appt_open &&
                   -1 != str_to_key_value_pairs(buffer, '=', key, READ_BUF_SIZE,
                                                value, READ_BUF_SIZE)) {
            /* Appt is open and we got a key-value pair */
            strip(key, strlen(key));
            strip(value, strlen(value));
            removequotes(value);

            /* TODO: handle errorous key-values here */
            appt_parse_properties(appt, key, value);
        }

    } /* /while */

    if (appt!=NULL)
        appt_destroy(appt);

    free(buffer);
    fclose(file);

    return success;
}

