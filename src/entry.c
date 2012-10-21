#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "entry.h"


#define INPUT_BUFFER_SIZE 128


bool entry_add (struct vector *entries)
{

    return true;
}


static int get_mandatory_input_str (char * str, size_t * size)
{
    ssize_t read;

    do {
        read = getline(&str, size, stdin);
    } while (read<2 && read!=-1);

    if (read != -1)
        str[read-1] = '\0';

    return read == -1 ? read : read-1;
}


static int get_optional_input_str (char * str, size_t * size)
{
    ssize_t read;

    read = getline(&str, size, stdin);

    if (read != -1)
        str[read-1] = '\0';

    return read == -1 ? read : read-1;

}


static bool is_numeric (const char *str)
{
    int i=0;
    while (str[i++]!='\0')
        if (str[i]<48 || str[i]>57)
            return false;

    return true;
}


static int get_optional_input_num (char * str, size_t * size)
{
    ssize_t read;

    do {
        read = getline(&str, size, stdin);
        if (read == -1)
            return -1;
        else if (read==1)
            return 0;
        str[read-1] = '\0';
    } while (!is_numeric(str));

    return read-1;
}


static void fill_time(struct tm * tm, char *buffer, size_t * size)
{
    time_t t;
    int retval;
    time(&t);
    *tm = *localtime(&t);

    printf("Year (default %d): ", 1900 + tm->tm_year);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        tm->tm_year = atoi(buffer) - 1900;

    printf("Month (default %d): ", 1 + tm->tm_mon);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        tm->tm_mon = atoi(buffer) - 1;

    printf("Day (default %d): ", tm->tm_mday);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        tm->tm_mday = atoi(buffer);

    printf("Hour (default %d): ", tm->tm_hour);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        tm->tm_hour = atoi(buffer);

    printf("Minute (default %d): ", tm->tm_min);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        tm->tm_min = atoi(buffer);

}


bool entry_add_interactive (struct vector *entries)
{
    char * buffer = malloc(INPUT_BUFFER_SIZE);
    size_t size = INPUT_BUFFER_SIZE;
    int read;
    struct entry *entry = entry_init();

    printf("Header:\n");
    read = get_mandatory_input_str(buffer, &size);
    if (read == -1) {
        goto clean_and_exit;
    } else {
        entry->header = malloc(read);
        strncpy(entry->header, buffer, read);
    }

    printf("Description:\n");
    read = get_optional_input_str(buffer, &size);
    if (read == -1) {
        goto clean_and_exit;
    } else if (read != 0) {
        entry->description = malloc(read);
        strncpy(entry->description, buffer, read);
    }

    fill_time(&(entry->start), buffer, &size);
    fill_time(&(entry->end), buffer, &size);

    vector_add(entries, (void *) entry);

#ifdef DEBUG
    printf("\nDone!\n");
    entry_dump(entry);
#endif

    free(buffer);
    return true;

clean_and_exit:
    free(buffer);
    entry_destroy(entry);
    return false;
}


struct entry *entry_init()
{
    time_t t = time(NULL);
    struct entry *entry = malloc(sizeof(struct entry));
    if (entry==NULL)
        return NULL;

    entry->header = NULL;
    entry->description = NULL;
    entry->category = NULL;

    localtime_r(&t, &entry->start);
    localtime_r(&t, &entry->end);

    return entry;
}


bool entry_save (FILE *file, struct entry *entry)
{
    fprintf(file, "ENTRY-START\n");
    fprintf(file, "header=\"%s\"\n", entry->header);

    if (entry->description!=NULL)
        fprintf(file, "description=\"%s\"\n", entry->description);

    if (entry->category!=NULL)
        fprintf(file, "category=\"%s\"\n", entry->category);

    fprintf(file, "start=%d\n", (int)mktime(&entry->start));
    fprintf(file, "end=%d\n", (int)mktime(&entry->end));

    fprintf(file, "ENTRY-END\n\n");

    return true;
}


void entry_destroy (struct entry *entry)
{
    free(entry->header);
    free(entry->description);
    free(entry->category);
    free(entry);
}


void entry_dump (struct entry *entry)
{
    size_t size = 32;
    char start[size];
    char end[size];

    strftime(start, size, "%F %H:%M", &entry->start);
    strftime(end, size, "%F %H:%M", &entry->end);

    printf ("%s -> %s\n", start, end);
    printf("%s\n", entry->header);

    if (entry->description!=NULL)
        printf("%s\n", entry->description);

    if (entry->category!=NULL)
        printf("Category: %s\n", entry->category);
}


