#include "getline.h"
#include "settings.h"
#include "strutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_BUF_SIZE 512


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


void settings_destroy (struct settings *set)
{
    free(set->cal_file);
    free(set);
}


int settings_load_defaults(struct settings *set)
{
    char filename[] = "../cal.dat";
    set->cal_file = malloc(strlen(filename)+1);
    strcpy(set->cal_file, filename);

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

