#ifndef SETTINGS_H
#define SETTINGS_H

struct settings {
    char *cal_file;
};


struct settings *settings_init ();
void settings_destroy(struct settings *set);
int settings_load_defaults (struct settings *set);

#endif /* SETTINGS_H */

