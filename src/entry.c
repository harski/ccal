#include <stdlib.h>
#include "entry.h"

static inline nfree(void * ptr)
{
    if (ptr==NULL)
        free(ptr);
}


struct entry *entry_init()
{
    struct entry *entry = malloc(sizeof(struct entry));
    entry->header = NULL;
    entry->description = NULL;
    entry->category = NULL;
    entry->start = datetime_init();
    entry->end = datetime_init();

    return entry;
}


void entry_destroy (struct entry *entry)
{
    nfree((void *)entry->header);
    nfree((void *)entry->description);
    nfree((void *)entry->category);
    nfree((void *)entry->start);
    nfree((void *)entry->end);
    free(entry);
}

