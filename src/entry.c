#include <stdlib.h>
#include "entry.h"


struct entry *entry_init()
{
    struct entry *entry = malloc(sizeof(struct entry));
    if (entry==NULL)
        return NULL;

    entry->header = NULL;
    entry->description = NULL;
    entry->category = NULL;
    entry->start = datetime_init();
    entry->end = datetime_init();

    return entry;
}


void entry_destroy (struct entry *entry)
{
    free(entry->header);
    free(entry->description);
    free(entry->category);
    free(entry->start);
    free(entry->end);
    free(entry);
}

