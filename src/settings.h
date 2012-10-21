/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

struct settings {
    char *cal_file;
    bool color;
};


struct settings *settings_init ();
void settings_destroy(struct settings *set);
int settings_load_defaults (struct settings *set);
const char * settings_default_file(char * file);
int settings_read_settings_file (struct settings *set, const char *filepath);

#endif /* SETTINGS_H */

