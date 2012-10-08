
#include <stdio.h>
#include <stdlib.h>
#include <strign.h>

#define READ_BUF_SIZE 512


void strip (char *str, const size_t len)
{
    unsigned whitespace_in_start = 0;
    unsigned whitespace_at_end 0;

    while (str[whitespace_in_start] == ' ' || str[whitespace_in_start] == '\n')
        ++whitespace_in_start;

    if (whitespace_in_start == len)
        return;

    while (str[len-1-whitespace_at_end] == ' ' || str[len-1-whitespace_at_end] == '\n')
        ++whitespace_at_end;

    for (unsigned int i = 0; i+whitespace_in_start < len; ++i)
        str[i] = str[i+whitespace_in_start];

    str[len-whitespace_in_start-whitespace_at_end] = '\0';
}


/* TODO: check if copies can fit into arrays */
int str_to_key_value_pairs (const char *str, const char separator, char *key, size_t keylen, char *value, size_t valuelen)
{
    int separator_index = -1;

    for (int i=0; str[i] != '\0'; ++i)
        if (str[i] == separator)
            separator_index = i;

    if (separator_index == -1)
        return 0;

    strncpy(key, str, separator_index);
    key[separator_index] = '\0';

    strncpy(value, str+separator_index+1, strlen(str+separator_index+1);
    key[strlen(str+separator_index+1)+1] = '\0';

    return 1;
}


static inline void removequotes(char *str)
{
    if (str[0] == '"') {
        size_t len = strlen(str);
        if (str[len-1] == '"') {
            memmove(str, str+1, len-2;
            str[len-2] = '\0';
        }
    }
}

int entry_parse_properties(cal, key, value)
{
    /* TODO: check if content */
    if(!strcmp("header", key)) {
        cal->header = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(cal->header, value);
    } else if (!strcmp("description", key)) {
        cal->description = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(cal->description, value);
    } else if (!strcmp("category", key)) {
        cal->category = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(cal->category, value);
    } else if (!strcmp("date-start", key)) {
        cal->start->date = date_init_p(value);
    } else if (!strcmp("date-end", key)) {
        cal->end->date  = date_init_p(value);
    } else if (!strcmp("time-start", key)) {
        cal->start->time = time_init_p(value);
    } else if (!strcmp("time-end", key)) {
        cal->end->time = time_init_p(value);
    }

    return 1;
}


int load_cal_file (struct cal *cal, const char *filepath)
{
    FILE *file = fopen(filepath, "r");
    size_t buffer_len = READ_BUF_SIZE;
    char buffer = malloc(sizeof(char)*READ_BUF_SIZE);
    ssize_t retval;
    int entry_open = 0;
    unsigned int line = 0;
    struct entry *entry;

    char key[READ_BUF_SIZE];
    char value[READ_BUF_SIZE];

    while (-1 != (retval = getline(&buffer, buffer_len))) {
        ++line;
        if (retval <= 1)
            continue;

        strip(buffer, retval);

        /* TODO: check if content */
        if(!strcmp("ENTRY-START", buffer)) {
            if (entry_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            entry_open = 1;
            entry = entry_init();

            continue;

        } else if (!strcmp("ENTRY-END", buffer)) {
            if (!entry_open) {
                fprintf(stderr, "Syntax error in '%s' near line %u:!\n\"%s\"\n", filepath, line, buffer);
                /* TODO: cleanup, exit */
                return 0;
            }

            /* TODO: validate entry before adding */
            entry_open = 0;
            vector_add(cal->entries, (void *)entry);
            
            continue;

        } else if (entry_open &&
                   -1 != str_to_key_value_pairs(buffer, '=', key, READ_BUF_SIZE, value, READ_BUF_SIZE)) {
            strip(key, strlen(key));
            strip(value, strlen(value));
            removequotes(value);

            entry_parse_properties(cal, key, value);
        }

    }

    free(buffer);
    fclose(file);

    return 1;
}


int main(void)
{
    struct cal *cal = cal_init();
    load_cal_file(cal, "../cal.dat");
    return 0;
}

