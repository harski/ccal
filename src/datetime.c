#include <stdio.h>
#include <stdlib.h>
#include "datetime.h"


struct datetime * datetime_init ()
{
    struct datetime *dt = malloc(sizeof(datetime));
    dt->date = NULL;
    dt->time = NULL;

    return dt;
}


/* TODO: check that buf is numeric */
struct date * date_init_p (const char *str)
{
    char buf[5];
    struct date *d = malloc(sizeof(struct date));

    snprintf(buf, str, 4);
    buf[4] = '\0';
    date->year = atoi(buf);

    snprintf(buf, str+4, 2);
    buf[2] = '\0';
    date->month = atoi(buf);

    snprintf(buf, str+6, 2);
    date->day = atoi(buf);

    return date;
}


struct time *time_init_p (const char *str)
{
    char buf[3];
    struct time *t = malloc(sizeof(struct time));

    buf[2] = '\0';

    snprintf(buf, str, 2);
    t->hour = atoi(buf);

    snprintf(buf, str+2, 2);
    t->minute = atoi(buf);

    return t;
}


void time_destroy (struct time *t)
{
    free(t);
}

