/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>

enum LogLevel {
    LL_ERROR,
    LL_WARNING,
    LL_INFO,
    LL_DEBUG
};

void do_log (enum LogLevel ll, const char *fmt, ...);
bool log_set_file (const char *filename);

#endif /* LOG_H */

