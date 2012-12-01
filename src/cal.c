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
#include "todo.h"

#define READ_BUF_SIZE 512


struct cal * cal_init ()
{
    struct cal *cal = malloc(sizeof(struct cal));

    if (cal==NULL)
        goto init_error;

    cal->appts = vector_init();
    cal->todos = vector_init();

    if (cal->appts==NULL || cal->todos==NULL)
        goto init_error;

    return cal;

init_error:
    free(cal->appts);
    free(cal->todos);
    free(cal);
    return NULL;
}


void cal_destroy (struct cal *cal)
{
    while (cal->appts->elements != 0)
        appt_destroy(vector_remove(cal->appts, cal->appts->elements -1));

    while (cal->todos->elements != 0)
        appt_destroy(vector_remove(cal->todos, cal->todos->elements -1));

    vector_destroy(cal->appts);
    vector_destroy(cal->todos);

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

    for (unsigned int i=0; i < cal->todos->elements; ++i)
        todo_save(file, (struct todo *) vector_get(cal->todos, i));

    fclose(file);
    return true;

}


int load_cal_file (struct cal *cal, const char *filepath)
{
    int success = 1;
    FILE *file;
    size_t buffer_len = READ_BUF_SIZE;
    char *buffer;
    int retval;
    bool appt_open = false;
    bool todo_open = false;
    unsigned int line = 0;
    struct appt *appt = NULL;
    struct todo *todo = NULL;

    char key[READ_BUF_SIZE];
    char value[READ_BUF_SIZE];

    buffer = malloc(sizeof(char)*READ_BUF_SIZE);
    if (buffer==NULL) {
        do_log(LL_ERROR, "Malloc failed in %s:%s", __FILE__, __func__);
        return 0;
    }

    /* Open the save file for reading */
    file = fopen(filepath, "r");
    if (file==NULL) {
       do_log(LL_WARNING, "%s", "Could not open save file for reading");
       return 0;
    }

    /* Loop through the save file */
    while (0 < (retval = getline_custom(&buffer, &buffer_len, file)) && success) {
        ++line;

        /* If line is empty, get next line */
        if (retval <= 1)
            continue;

        strip(buffer, retval);

        /* check if content */
        if(!strcmp("APPT-START", buffer)) {
            if (appt_open || todo_open) {
                do_log(LL_WARNING, "Syntax error in '%s' near line %u:\n\"%s\"",
                       filepath, line, buffer);
                success = 0;
            } else {
                appt_open = true;
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
                appt_open = false;
                appt = NULL;
            } else {
                /* The parsed appt entry does not validate: dump it */
                do_log(LL_WARNING, "Invalid appt in savefile, near line %d", line);
                success = 0;
            }
        } else if (!strcmp("TODO-START", buffer)) {
            if (appt_open || todo_open) {
                do_log(LL_WARNING, "Syntax error in '%s' near line %u:\n\"%s\"",
                       filepath, line, buffer);
                success = 0;
            } else {
                todo_open = true;
                todo = todo_init();
            }
        } else if (!strcmp("TODO-END", buffer)) {
            /* Check that todo is opened before it is closed */
            if (!todo_open) {
                do_log(LL_WARNING, "Syntax error in '%s' near line %u:\n\"%s\"",
                       filepath, line, buffer);
                success = 0;
            } else if (todo_validate(todo)) {
                /* parsing todo-entry succesfull */
                vector_add(cal->todos, (void *) todo);
                todo_open = false;
                todo = NULL;
            } else {
                /* todo did not validate */
                do_log(LL_WARNING, "Invalid todo in savefile, near line %d", line);
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
        } else if (todo_open &&
                   -1 != str_to_key_value_pairs(buffer, '=', key, READ_BUF_SIZE,
                                                value, READ_BUF_SIZE)) {
            /* Appt is open and we got a key-value pair */
            strip(key, strlen(key));
            strip(value, strlen(value));
            removequotes(value);

            /* TODO: handle errorous key-values here */
            todo_parse_properties(todo, key, value);
        }


    } /* /while */

    if (appt!=NULL)
        appt_destroy(appt);

    free(buffer);
    fclose(file);

    return success;
}

