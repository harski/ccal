/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef CAL_H
#define CAL_H

#include "vector.h"
#include <stdbool.h>


struct cal {
    struct vector *appts;
    struct vector *todos;
};

struct cal * cal_init ();
void cal_destroy (struct cal *cal);

void cal_dump (const struct cal *cal);
bool cal_save (const struct cal *cal, const char *filename);

int load_cal_file (struct cal *cal, const char *filepath);

#endif /* CAL_H */

