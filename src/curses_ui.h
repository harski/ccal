/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef CURSES_UI_H
#define CURSES_UI_H

#include "cal.h"
#include "settings.h"
#include "../config.h"

#ifdef HAVE_NCURSESW_CURSES_H
# include <ncursesw/ncurses.h>
#elif HAVE_NCURSESW_H
# include <ncursesw.h>
#endif

int ui_show_dump (WINDOW **wins, struct settings *set, struct cal *cal);
int ui_show_main_view (struct settings *set, struct cal *cal);

#endif /* CURSES_UI_H */

