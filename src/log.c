/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "log.h"

#define FILE_PATH_SIZE 128
#define TIME_STR_SIZE 64

static char _log_file[FILE_PATH_SIZE] = {'\0'};
static bool _log_open = false;
static enum LogLevel _log_level = LL_WARNING;
//static FILE * _log_handle;


static inline const char * _get_ll_str(enum LogLevel ll)
{
    const char *str;

    switch (ll) {
    case LL_ERROR:
        str = "ERROR";
        break;
    case LL_WARNING:
        str = "WARNING";
        break;
    case LL_INFO:
        str = "INFO";
        break;
    case LL_DEBUG:
        str = "DEBUG";
        break;
    default:
        str = "UNKNOWN";
        break;
    }

    return str;
}

void do_log (enum LogLevel ll, const char *fmt, ...)
{
    va_list ap;
    FILE *file;
    bool fallback = false;
    char time_str[TIME_STR_SIZE];
    const char *log_level_str;
    time_t stamp;
    struct tm tm;

    /* return quietly if loglevel is not high enough */
    if (ll > _log_level)
        return;

    /* Get timestamp as char* */
    stamp = time(NULL);
    localtime_r(&stamp, &tm);
    strftime(time_str, TIME_STR_SIZE-1, "%F-%T", &tm);

    log_level_str = _get_ll_str(ll);

    /* TODO: thread-safe */
    /* If log file is not set or cannot open the _log_file, log to
     * stderr. Otherwise to the _logfile */
    if (_log_file[0]=='\0' || _log_open) {
        fallback = true;
    } else {
        _log_open = true;
        file = fopen(_log_file, "a");

        if (file != NULL) {
            fprintf(file, "%s:%s: ", time_str, log_level_str);
            va_start (ap, fmt);
            vfprintf (file, fmt, ap);
            va_end (ap);
            fprintf(file, "\n");

            fclose(file);
        } else {
            fallback = true;
        }

        _log_open = false;
    }

    /* If valid logfile is not set or it cannot be opened, fallback to
     * printing to stderr */
    if (fallback) {
        fprintf(stderr, "%s:%s: ", time_str, log_level_str);
        va_start (ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end (ap);
        fprintf(stderr, "\n");
    }
}

bool log_set_file (const char *filename)
{
    bool valid;
    FILE *handle = fopen(filename, "a");

    if (strlen(filename) >= FILE_PATH_SIZE || handle == NULL) {
        valid = false;
    } else {
        valid = true;
        fclose(handle);

        strncpy(_log_file, filename, FILE_PATH_SIZE);
    }

    return valid;
}

