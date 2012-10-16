#ifndef ENTRY_H
#define ENTRY_H

#include <stdbool.h>
#include <time.h>
#include <vector.h>

struct entry {
    char *header;
    char *description;
    char *category;
    struct tm start;
    struct tm end;
};


struct entry *entry_init();
void entry_destroy (struct entry *entry);

bool entry_add_interactive (struct vector *entries);
void entry_dump (struct entry *entry);
bool entry_save (FILE *file, struct entry *entry);

#endif /* ENTRY_H */

