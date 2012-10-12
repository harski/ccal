#include <string.h>
#include <stdlib.h>
#include "datetime.h"


struct datetime * datetime_init ()
{
    struct datetime *dt = malloc(sizeof(struct datetime));
    dt->date = NULL;
    dt->time = NULL;

    return dt;
}


void datetime_destroy(struct datetime *date)
{
    date_destroy(date->date);
    time_destroy(date->time);
    free(date);
}


/* TODO: check that buf is numeric */
struct date * date_init_p (const char *str)
{
    char buf[5];
    struct date *d = malloc(sizeof(struct date));

    strncpy(buf, str, 4);
    buf[4] = '\0';
    d->year = atoi(buf);

    strncpy(buf, str+4, 2);
    buf[2] = '\0';
    d->month = atoi(buf);

    strncpy(buf, str+6, 2);
    d->day = atoi(buf);

    return d;
}


void date_destroy (struct date * date)
{
    free(date);
}


struct time *time_init_p (const char *str)
{
    char buf[3];
    struct time *t = malloc(sizeof(struct time));

    buf[2] = '\0';

    strncpy(buf, str, 2);
    t->hour = atoi(buf);

    strncpy(buf, str+2, 2);
    t->minute = atoi(buf);

    return t;
}


void time_destroy (struct time *t)
{
    free(t);
}

