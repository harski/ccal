/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef CURSES_UI_H
#define CURSES_UI_H

#include "cal.h"
#include "settings.h"
#include <ncurses.h>


int ui_show_dump (WINDOW **wins, struct settings *set, struct cal *cal);
int ui_show_main_view (struct settings *set, struct cal *cal);

#endif /* CURSES_UI_H */

