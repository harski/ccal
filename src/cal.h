#include <vector.h>


struct cal {
    struct vector *entries;
};

struct cal * cal_init ();
void cal_destroy (struct cal *cal);

