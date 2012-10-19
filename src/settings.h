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
int settings_read_settings_file (struct settings *set, const char *filepath);

#endif /* SETTINGS_H */

