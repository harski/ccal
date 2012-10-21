/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include "config.h"
#include "curses_ui.h"
#include "entry.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


static void print_main_header (WINDOW * win, const struct settings *set);
static void update_top_bar (WINDOW * win, const struct settings *set,
                            const char *str);
static int ui_show_day_agenda (WINDOW *win, const struct settings *set,
                               const struct cal *cal);

static void print_main_header (WINDOW * win, const struct settings *set)
{
    update_top_bar (win, set, "q:Quit  d:Dump");
}


void ui_init (struct settings *set)
{
    initscr();
    cbreak();
    noecho();

    if (has_colors() && set->color) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLUE);
    }
    debug_print("has_colors() = '%d'\n", has_colors());
}


static int ui_show_day_agenda (WINDOW *win, const struct settings *set,
                               const struct cal *cal)
{
    time_t t_now = time(NULL);
    struct tm *tm = localtime(&t_now);
    char * time_str;
    int winx, winy;
    int entry_start_line = 3;
    int header_start_col = 14;
    struct vector *entries = cal->entries;
    struct entry *entry;

    getmaxyx(win, winy, winx);
    werase(win);

    update_top_bar(NULL, set, "q:Quit");

    time_str = malloc(winy+1);
    strftime(time_str, winy+1, "%A %x", tm);

    mvwprintw(win, 2, 0, time_str);

    if (entries->elements > 0) {
        for(int i=0; i<entries->elements; ++i) {
            entry = (struct entry *) vector_get(entries, i);
            char date[13];
            snprintf(date, 13, "%.2d:%.2d-%.2d:%.2d:", entry->start.tm_hour,
                     entry->start.tm_min, entry->end.tm_hour, entry->end.tm_min);
            mvwprintw (win, entry_start_line+i+1, 0, "%s", date);
            mvwprintw (win, entry_start_line+i+1, header_start_col, "%s", entry->header);
        }
    } else {
        mvwprintw(win, 4, 3, "Nothing to do today :)");
    }

    wrefresh(win);

    free(time_str);
    return 1;
}


int ui_show_dump (struct settings *set, struct cal *cal)
{
    struct vector *entries = cal->entries;
    WINDOW *d_win = newwin(LINES, COLS, 0, 0);
    char select;
    bool exit = false;

    for (int i = 0; i<entries->elements; ++i) {
        struct entry * tmp = vector_get(entries, i);
        mvwprintw(d_win, i+1, 3, "%s", tmp->header);
    }

    wmove(d_win, 0, 0);
    wrefresh(d_win);

    while (!exit) {
        select = wgetch(d_win);
        switch (select) {

        case 'q':
            exit = true;
            break;

        default:
            break;
        }
        wrefresh(d_win);
    }

    delwin(d_win);

    return 1;
}


int ui_show_main_view (struct settings *set, struct cal *cal)
{
    bool exit = false;
    char select;
    WINDOW * main_win;

    ui_init(set);

    print_main_header(NULL, set);

    main_win = newwin(LINES-1, COLS, 1, 0);

    ui_show_day_agenda(main_win, set, cal);

    while (!exit) {
        select = wgetch(main_win);
        switch (select) {
        case 'd':
            ui_show_dump(set, cal);
            break;
        case 'q':
            exit = true;
            break;
        default:
            break;
        }
        wrefresh(main_win);
    }

    delwin(main_win);
    endwin();

    return 1;
}


static void update_top_bar (WINDOW * win, const struct settings *set,
                               const char *str)
{
    bool m_all;
    if (win==NULL) {
        win = newwin(1, COLS, 0, 0);
        if (win==NULL) {
            fprintf(stderr, "Error in creating main header win\n");
            return;
        }
        m_all = true;
    }

    if (set->color) {
        wattron(win, COLOR_PAIR(1));
        for (int i = 0; i<COLS; ++i)
            mvwprintw(win, 0, i, " ");
    }
    mvwprintw(win, 0, 0, str);

    if (set->color)
        wattroff(win, COLOR_PAIR(1));

    wrefresh(win);

    if (m_all)
        delwin(win);
}

