
#include <stdlib.h>
#include "cal.h"
#include "entry.h"


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
    while (cal->entries->elements != 0) {
        entry_destroy(vector_remove(cal->entries, cal->entries->elements -1));
    }

    vector_destroy(cal->entries);
    free(cal);
}

