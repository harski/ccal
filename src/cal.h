#ifndef CAL_H
#define CAL_H

#include <vector.h>


struct cal {
    struct vector *entries;
};

struct cal * cal_init ();
void cal_destroy (struct cal *cal);

void cal_dump (const struct cal *cal);

#endif /* CAL_H */

