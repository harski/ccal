#include <stdio.h>
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


void entry_dump (struct entry *entry)
{
    char start[20];
    char end[20];

    datetime_to_string(entry->start, start);
    datetime_to_string(entry->end, end);

    printf("%s\n", entry->header);
    printf("%s\n", entry->description);

    if (entry->category!=NULL)
        printf("Category: %s\n", entry->category);

    printf("%s -> %s\n", start, end);
}


