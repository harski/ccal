#ifndef CAL_H
#define CAL_H

#include <stdbool.h>
#include <vector.h>


struct cal {
    struct vector *entries;
};

struct cal * cal_init ();
void cal_destroy (struct cal *cal);

void cal_dump (const struct cal *cal);
bool cal_save (const struct cal *cal, const char *filename);

#endif /* CAL_H */

