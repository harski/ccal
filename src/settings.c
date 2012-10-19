#include <stdlib.h>
#include <string.h>
#include "settings.h"


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

    set->colors = true;

    return 1;
}

