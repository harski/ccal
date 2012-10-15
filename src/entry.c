#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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


static void fill_time(struct datetime * dt, char *buffer, size_t * size)
{
    time_t t;
    struct tm *tm;
    int retval;
    time(&t);

    tm = localtime(&t);

    printf("\nYear (default %d): ", 1900+tm->tm_year);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        dt->date->year = atoi(buffer);
    else
        dt->date->year = 1900 + tm->tm_year;

    printf("\nMonth (default %d): ", 1+tm->tm_mon);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        dt->date->month = atoi(buffer);
    else
        dt->date->month = 1 + tm->tm_mon;

    printf("\nDay (default %d): ", tm->tm_mday);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        dt->date->day = atoi(buffer);
    else
        dt->date->day = tm->tm_mday;

    printf("\nHour (default %d): ", tm->tm_hour);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        dt->time->hour = atoi(buffer);
    else
        dt->time->hour = tm->tm_hour;

    printf("\nMinute (default %d): ", tm->tm_min);
    retval = get_optional_input_num(buffer, size);
    if (retval >0)
        dt->time->minute = atoi(buffer);
    else
        dt->time->minute = tm->tm_min;

}


bool entry_add_interactive (struct vector *entries)
{
    char * buffer = malloc(INPUT_BUFFER_SIZE);
    size_t size = INPUT_BUFFER_SIZE;
    int read;
    struct entry *entry = entry_init();
    entry->start->date = date_init();
    entry->start->time = time_init();
    entry->end->date = date_init();
    entry->end->time = time_init();

    printf("Header:\n");
    read = get_mandatory_input_str(buffer, &size);
    if (read == -1) {
        goto clean_and_exit;
    } else {
        entry->header = malloc(read);
        strncpy(entry->header, buffer, read);
    }

    printf("\nDescription:\n");
    read = get_optional_input_str(buffer, &size);
    if (read == -1) {
        goto clean_and_exit;
    } else if (read != 0) {
        entry->description = malloc(read);
        strncpy(entry->description, buffer, read);
    }

    fill_time(entry->start, buffer, &size);
    fill_time(entry->end, buffer, &size);

    vector_add(entries, (void *) entry);

    free(buffer);
    return true;

clean_and_exit:
    free(buffer);
    entry_destroy(entry);
    return false;
}


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

    fprintf(file, "ENTRY-END\n\n");

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


