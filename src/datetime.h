#ifndef DATETIME_H
#define DATETIME_H

struct date {
    int year;
    int month;
    int day;
};

struct time {
    int hour;
    int minute;
};


struct datetime {
    struct date *date;
    struct time *time;
};

struct datetime * datetime_init();
void datetime_destroy(struct datetime *date);

/* Expects str to be at least 17 chars */
void datetime_to_string (struct datetime *dt, char *str);

struct date * date_init ();
struct date * date_init_p (const char *str);
void date_destroy (struct date * date);

struct time *time_init ();
struct time *time_init_p (const char *str);
void time_destroy (struct time *t);

#endif /* DATETIME_H */

