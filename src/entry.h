#ifndef ENTRY_H
#define ENTRY_H

#include <stdbool.h>
#include <vector.h>
#include "datetime.h"

struct entry {
    char *header;
    char *description;
    char *category;
    struct datetime *start;
    struct datetime *end;
};


struct entry *entry_init();
void entry_destroy (struct entry *entry);

void entry_dump (struct entry *entry);
bool entry_save (FILE *file, const struct entry *entry);

#endif /* ENTRY_H */

