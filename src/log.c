#include <stdio.h>
#include "log.h"


void wlog (const char *path, const char *fmt, ...)
{
    char *logfile;
    va_list ap;
    FILE *file;

    if (path==NULL)
        logfile = "ccal.log";
    else
        logfile = path;

    file = fopen(logfile, "a");

    if (file==NULL)
        return;

    va_start (ap, fmt);
    vfprintf (file, fmt, ap);
    va_end (ap);

    fclose(file);
}

