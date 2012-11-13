/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "appt.h"


#define INPUT_BUFFER_SIZE 128


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


bool appt_add_interactive (struct vector *appts)
{
    char * buffer = malloc(INPUT_BUFFER_SIZE);
    size_t size = INPUT_BUFFER_SIZE;
    int read;
    struct appt *appt = appt_init();

    printf("Header:\n");
    read = get_mandatory_input_str(buffer, &size);
    if (read == -1) {
        goto clean_and_exit;
    } else {
        appt->header = malloc(read);
        strncpy(appt->header, buffer, read);
    }

    printf("Description:\n");
    read = get_optional_input_str(buffer, &size);
    if (read == -1) {
        goto clean_and_exit;
    } else if (read != 0) {
        appt->description = malloc(read);
        strncpy(appt->description, buffer, read);
    }

    fill_time(&(appt->start), buffer, &size);
    fill_time(&(appt->end), buffer, &size);

    vector_add(appts, (void *) appt);

#ifdef DEBUG
    printf("\nDone!\n");
    appt_dump(appt);
#endif

    free(buffer);
    return true;

clean_and_exit:
    free(buffer);
    appt_destroy(appt);
    return false;
}


struct appt *appt_init()
{
    time_t t = time(NULL);
    struct appt *appt = malloc(sizeof(struct appt));
    if (appt==NULL)
        return NULL;

    appt->header = NULL;
    appt->description = NULL;
    appt->category = NULL;

    localtime_r(&t, &appt->start);
    localtime_r(&t, &appt->end);

    return appt;
}


int appt_parse_properties (struct appt *appt, char *key, char *value)
{
    /* check if content */
    if(!strcmp("header", key)) {
        appt->header = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(appt->header, value);
    } else if (!strcmp("description", key)) {
        appt->description = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(appt->description, value);
    } else if (!strcmp("category", key)) {
        appt->category = malloc(sizeof(char)*(strlen(value)+1));
        strcpy(appt->category, value);
    } else if (!strcmp("start", key)) {
        time_t t = (time_t) atoi(value);
        appt->start = *localtime(&t);
    } else if (!strcmp("end", key)) {
        time_t t = (time_t) atoi(value);
        appt->end = *localtime(&t);
    } else {
        fprintf(stderr, "Error parsing calfile: key '%s' isn't a property!\n", key);
    }

    return 1;
}


bool appt_save (FILE *file, struct appt *appt)
{
    fprintf(file, "ENTRY-START\n");
    fprintf(file, "header=\"%s\"\n", appt->header);

    if (appt->description!=NULL)
        fprintf(file, "description=\"%s\"\n", appt->description);

    if (appt->category!=NULL)
        fprintf(file, "category=\"%s\"\n", appt->category);

    fprintf(file, "start=%d\n", (int)mktime(&appt->start));
    fprintf(file, "end=%d\n", (int)mktime(&appt->end));

    fprintf(file, "ENTRY-END\n\n");

    return true;
}


void appt_destroy (struct appt *appt)
{
    free(appt->header);
    free(appt->description);
    free(appt->category);
    free(appt);
}


void appt_dump (struct appt *appt)
{
    size_t size = 32;
    char start[size];
    char end[size];

    strftime(start, size, "%F %H:%M", &appt->start);
    strftime(end, size, "%F %H:%M", &appt->end);

    printf ("%s -> %s\n", start, end);
    printf("%s\n", appt->header);

    if (appt->description!=NULL)
        printf("%s\n", appt->description);

    if (appt->category!=NULL)
        printf("Category: %s\n", appt->category);
}


bool appt_validate (const struct appt *appt)
{
    if (appt->header==NULL) 
        return false;

    return true;
}

