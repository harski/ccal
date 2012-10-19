#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector.h>
#include <unistd.h>

#include "action.h"
#include "cal.h"
#include "curses_ui.h"
#include "entry.h"
#include "getline.h"
#include "settings.h"
#include "strutils.h"

#define READ_BUF_SIZE 512


int entry_parse_properties (struct entry *entry, char *key, char *value)
{
    /* check if content */
    if(!strcmp("header", key)) {
        entry->header = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(entry->header, value);
    } else if (!strcmp("description", key)) {
        entry->description = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(entry->description, value);
    } else if (!strcmp("category", key)) {
        entry->category = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(entry->category, value);
    } else if (!strcmp("start", key)) {
        time_t t = (time_t) atoi(value);
        entry->start = *localtime(&t);
    } else if (!strcmp("end", key)) {
        time_t t = (time_t) atoi(value);
        entry->end = *localtime(&t);
    } else {
        fprintf(stderr, "Error parsing calfile: key '%s' isn't a property!\n", key);
    }

    return 1;
}


int load_cal_file (struct cal *cal, const char *filepath)
{
    FILE *file = fopen(filepath, "r");
    size_t buffer_len = READ_BUF_SIZE;
    char *buffer = malloc(sizeof(char)*READ_BUF_SIZE);
    int retval;
    int entry_open = 0;
    unsigned int line = 0;
    struct entry *entry;

    char key[READ_BUF_SIZE];
    char value[READ_BUF_SIZE];

    while (0 < (retval = getline_custom(&buffer, &buffer_len, file))) {
        ++line;
        if (retval <= 1)
            continue;

        strip(buffer, retval);

        /* TODO: check if content */
        if(!strcmp("ENTRY-START", buffer)) {
            if (entry_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            entry_open = 1;
            entry = entry_init();

            continue;

        } else if (!strcmp("ENTRY-END", buffer)) {
            if (!entry_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            /* TODO: validate entry before adding */
            entry_open = 0;
            vector_add(cal->entries, (void *)entry);

            continue;

        } else if (entry_open &&
                   -1 != str_to_key_value_pairs(buffer, '=', key, READ_BUF_SIZE, value, READ_BUF_SIZE)) {
            strip(key, strlen(key));
            strip(value, strlen(value));
            removequotes(value);

            entry_parse_properties(entry, key, value);
        }

    }

    free(buffer);
    fclose(file);

    return 1;
}


int main(int argc, char *argv[])
{
    struct settings *set = settings_init();
    struct cal *cal = cal_init();
    enum Action action = ACTION_NOT_SET;
    int opt;
    char set_file[128];

    settings_default_file(set_file);

    while ((opt = getopt(argc, argv, "f:da")) != -1) {
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

        default:
            fprintf(stderr, "Option '%s' is not recognized!\nQuitting...\n", optarg);
            exit(1);
        }
    }

    settings_read_settings_file(set, set_file);
    load_cal_file(cal, set->cal_file);

    switch (action) {
    case ACTION_DUMP:
        cal_dump(cal);
        break;

    case ACTION_ADD:
        entry_add_interactive(cal->entries);
        cal_save(cal, set->cal_file);
        break;

    default:
        ui_show_main_view(set, cal);
        fprintf(stderr, "No action set: Quitting...\n");
    }

    settings_destroy(set);
    cal_destroy(cal);
    return 0;
}

