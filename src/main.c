/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include "config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector.h>
#include <unistd.h>
#include <locale.h>

#include "action.h"
#include "cal.h"
#include "curses_ui.h"
#include "appt.h"
#include "getline.h"
#include "settings.h"
#include "strutils.h"

#define READ_BUF_SIZE 512


int appt_parse_properties (struct appt *appt, char *key, char *value)
{
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
    }

    return 1;
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
        if (retval <= 1)
            continue;

        strip(buffer, retval);

        /* TODO: check if content */
        if(!strcmp("ENTRY-START", buffer)) {
            if (appt_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            appt_open = 1;
            appt = appt_init();

            continue;

        } else if (!strcmp("ENTRY-END", buffer)) {
            if (!appt_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            /* TODO: validate appt before adding */
            appt_open = 0;
            vector_add(cal->appts, (void *)appt);

            continue;

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


void print_version()
{
    printf("%s %s\n", NAME, VERSION);
}


int main(int argc, char *argv[])
{
    struct settings *set = settings_init();
    struct cal *cal = cal_init();
    enum Action action = ACTION_NOT_SET;
    int opt;
    char set_file[128];

    setlocale(LC_CTYPE, "");

    settings_default_file(set_file);

    while (true) {
        int this_optind = optind ? optind : 1;
        int option_index = 0;

        static struct option long_options[] = {
            {"add",     no_argument,    0,  'a'},
            {"dump",    no_argument,    0,  'd'},
            {"version", no_argument,    0,  'V'}
        };

        opt = getopt_long(argc, argv, "f:daV", long_options, &option_index);

        if (opt==-1)
            break;

        switch (opt) {
        case 'a':
            action = ACTION_ADD;
            break;

        case 'f':
            set->cal_file = optarg;
            break;

        case 'd':
            action = ACTION_DUMP;
            break;

        case 'V':
            action = ACTION_PRINT_VERSION;
            break;

        default:
            fprintf(stderr, "Option '%s' is not recognized!\nQuitting...\n", optarg);
            exit(1);
        }
    }

    /* TODO: fix this mess of reading settings from file and accepting them
     * from cli: cli should take precedence! */
    settings_read_settings_file(set, set_file);
    load_cal_file(cal, set->cal_file);

    switch (action) {
    case ACTION_DUMP:
        cal_dump(cal);
        break;

    case ACTION_ADD:
        appt_add_interactive(cal->appts);
        cal_save(cal, set->cal_file);
        break;

    case ACTION_PRINT_VERSION:
        print_version();
        break;

    default:
        ui_show_main_view(set, cal);
    }

    settings_destroy(set);
    cal_destroy(cal);
    return 0;
}

