/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef ENTER_H
#define ENTER_H

#define _XOPEN_SOURCE_EXTENDED

#include "../config.h"

#ifdef HAVE_NCURSESW_CURSES_H
# include <ncursesw/ncurses.h>
#elif HAVE_NCURSESW_H
# include <ncursesw.h>
#endif

#include <stddef.h>
#include <time.h>

/* *str must be properly allocated and NUL terminated */
int ui_edit_string (WINDOW *win, const int row, const int col,
                   const char *prompt, char **str, size_t *size);

int ui_get_date (WINDOW *win, const int row, const int col,
                 const char *prompt, struct tm *tm);

int ui_get_string (WINDOW *win, const int row, const int col,
                   const char *prompt, char **str, size_t *size);

bool ui_get_yes_no (WINDOW *win, const int row, const int col, const char *prompt, const char def);


#endif

