#ifndef ENTRY_H
#define ENTRY_H

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

#endif /* ENTRY_H */

