/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include "getline.h"
#include "settings.h"
#include "strutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_BUF_SIZE 512


static char * get_default_cal_file()
{
    char * file;
    size_t len, tmplen;
    const char fn[] = ".ccal.dat";

    len = strlen(getenv("HOME")) + strlen(fn) + 2;
    file = malloc(len);
    strcpy(file, getenv("HOME"));

    tmplen = strlen(file);

    if (file[tmplen-1] != '/') {
        file[tmplen] = '/';
        file[tmplen+1] = '\0';
        ++len;
    }

    strcat(file, fn);
    return file;
}


static void scan_val (struct settings *set, const char *key, const char *val)
{
    if (!strcmp(key, "color")) {
        if (!strcmp(val, "true"))
            set->color = true;
        else if (!strcmp(val, "false"))
            set->color = false;
        else
            fprintf(stderr, "'%s' is not a valid value for setting color\n", val);
    }
}


struct settings * settings_init()
{
    struct settings *set = malloc(sizeof(struct settings));
    settings_load_defaults(set);
    return set;
}


const char * settings_default_file(char * file)
{
    size_t len;
    strcpy(file, getenv("HOME"));
    len = strlen(file);

    if (file[len-1] != '/') {
        file[len] = '/';
        file[len+1] = '\0';
        ++len;
    }

    strcat(file, ".ccalrc");
    return file;
}


void settings_destroy (struct settings *set)
{
    free(set->cal_file);
    free(set);
}


int settings_load_defaults(struct settings *set)
{
    set->cal_file = get_default_cal_file();
    set->color = true;

    return 1;
}


int settings_read_settings_file (struct settings *set, const char *filepath)
{
    char key[READ_BUF_SIZE];
    char value[READ_BUF_SIZE];
    size_t buffer_len = READ_BUF_SIZE;
    char * buffer = malloc(READ_BUF_SIZE);
    int retval;

    FILE *file = fopen(filepath, "r");

    if (file==NULL)
        return 0;

    while (0 < (retval = getline_custom(&buffer, &buffer_len, file))) {
        if (!str_to_key_value_pairs(buffer, '=', key, READ_BUF_SIZE, value, READ_BUF_SIZE))
            continue;

        strip(key, strlen(key));
        strip(value, strlen(value));

        scan_val(set, key, value);
    }

    fclose(file);
    return 1;
}

