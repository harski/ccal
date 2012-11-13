/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include "config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "action.h"
#include "cal.h"
#include "curses_ui.h"
#include "appt.h"
#include "settings.h"


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

