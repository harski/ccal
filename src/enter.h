/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef ENTER_H
#define ENTER_H

#include <ncurses.h>
#include <stddef.h>

int ui_get_string (WINDOW *win, const int row, const int col,
                   const char *prompt, char **str, size_t *size);


#endif

