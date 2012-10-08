
#include <stdlib.h>
#include "cal.h"


struct cal * cal_init ()
{
    struct cal *cal = malloc(sizeof(struct cal));

    if (cal==NULL)
        return NULL;

    cal->entries = vector_create();
    
    if (cal->entries == NULL) {
        free(cal);
        return NULL;
    }
    
    return cal;
}


void cal_destroy (struct cal *cal)
{
    vector_destroy_all(cal->entries);
    free(cal);
}

