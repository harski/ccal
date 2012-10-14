#include "config.h"

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


bool entry_save (FILE *file, const struct entry *entry)
{
    fprintf(file, "ENTRY-START\n");
    fprintf(file, "header=\"%s\"\n", entry->header);

    if (entry->description!=NULL)
        fprintf(file, "description=\"%s\"\n", entry->description);

    if (entry->start != NULL) {
        if (entry->start->date != NULL) {
            fprintf(file, "start-date=%d%d%d\n", entry->start->date->year,
                    entry->start->date->month, entry->start->date->day);
        }

        if (entry->start->time != NULL) {
            fprintf(file, "start-time=%d%d\n", entry->start->time->hour,
                    entry->start->time->minute);
        }
    }

    if (entry->end != NULL) {
        if (entry->end->date != NULL) {
            fprintf(file, "end-date=%d%d%d\n", entry->end->date->year,
                    entry->end->date->month, entry->end->date->day);
        }

        if (entry->end->time != NULL) {
            fprintf(file, "end-time=%d%d\n", entry->end->time->hour,
                    entry->end->time->minute);
        }
    }

    fprintf(file, "ENTRY-END\n");

    return true;
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

    if (entry->description!=NULL)
        printf("%s\n", entry->description);

    if (entry->category!=NULL)
        printf("Category: %s\n", entry->category);

    if (start[0]!='\0')
        printf("%s", start);

    if (end[0]!='\0')
        printf(" -> %s\n", end);
    else
        printf("\n");
}


