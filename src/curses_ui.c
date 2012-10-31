/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE

#include "config.h"
#include "curses_ui.h"
#include "enter.h"
#include "entry.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


enum ColorPairs {
    CP_HEADER = 1,
    CP_CONTENT
};

static void update_top_bar (WINDOW * win, const struct settings *set,
                            const char *str);
static int ui_add_entry (WINDOW *win, struct settings *set,
                  struct cal *cal);
static void ui_init_color(const struct settings *set);
static int ui_show_day_agenda (WINDOW *win, const struct tm *day, const struct settings *set,
                               const struct cal *cal);
static bool entry_is_today(const struct entry *entry, const struct tm *tm);
static inline struct tm *get_today();
static inline void next_day (struct tm *tm);
static inline void prev_day (struct tm *tm);
static bool prompt_for_save (const struct settings *set);
static bool same_day (const struct tm *t1, const struct tm *t2);


static bool entry_is_today(const struct entry *entry, const struct tm *tm)
{
    if (same_day(&(entry->start), tm) || same_day(&(entry->end), tm))
        return true;

    return false;
}


static inline struct tm *get_today()
{
    struct tm * tm =  malloc(sizeof(struct tm));
    time_t tmp_time = time(NULL);
    return localtime_r(&tmp_time, tm);
}


static inline void next_day (struct tm *tm)
{
    time_t now = mktime(tm);
    now += 60*60*24;
    localtime_r(&now, tm);
}


static inline void prev_day (struct tm *tm)
{
    time_t now = mktime(tm);
    now -= 60*60*24;
    localtime_r(&now, tm);

}


bool prompt_for_save (const struct settings *set)
{
    int input;
    bool input_ok = false;
    bool ans;
    WINDOW *win = newwin(1, COLS, LINES-1, 0);

    if (set->color)
        wbkgd(win, A_NORMAL|COLOR_PAIR(CP_CONTENT));

    werase(win);

    mvwprintw(win, 0, 0, "Save changes to cal? [Y/n]: ");
    wrefresh(win);

    do {
        input = wgetch(win);
        if (input < 128) {
            if (input=='n') {
                input_ok = true;
                ans = false;
            } else if (input=='y' || input=='Y') {
                input_ok = true;
                ans = true;
            }
        }
    } while (!input_ok);

    delwin(win);

    return ans;
}


static bool same_day (const struct tm *t1, const struct tm *t2)
{
    if (t1->tm_year == t2->tm_year && t1->tm_mon == t2->tm_mon &&
        t1->tm_mday == t2->tm_mday)
        return true;

    return false;
}


static int ui_add_entry (WINDOW *win, struct settings *set,
                  struct cal *cal)
{
    struct entry * entry = entry_init();
    size_t tmp_size = 128;
    char *tmp = malloc(tmp_size);
    int success = 1;
    int line = 2;

    update_top_bar(NULL, set, "q:Cancel  return:Select  s:Save");
    werase(win);

    ui_get_string(win, line++, 0, "Header", &tmp, &tmp_size);
    entry->header = malloc(1+strlen(tmp));
    strcpy(entry->header, tmp);
    tmp[0] = '\0';

    ui_get_string(win, line++, 0, "start time (yymmdd hhmm)", &tmp, &tmp_size);
    strptime(tmp, "%y%m%d %H%M", &entry->start);
    tmp[0] = '\0';

    ui_get_string(win, line++, 0, "end time (yymmdd hhmm)", &tmp, &tmp_size);
    strptime(tmp, "%y%m%d %H%M", &entry->end);

    if (entry_validate(entry)) {
        vector_add(cal->entries, entry);
        set->cal_changed = true;
        success = 1;
    } else {
        /* TODO: report error */
        success = 0;
    }

    return success;
}


void ui_init (struct settings *set)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors() && set->color)
        ui_init_color(set);
}


static void ui_init_color(const struct settings *set) {
    start_color();
    init_pair(CP_HEADER, COLOR_YELLOW, COLOR_BLUE);
    init_pair(CP_CONTENT, COLOR_WHITE, COLOR_BLACK);
}


static inline void print_agenda_day_entry (WINDOW *win, const struct entry *entry, unsigned int index)
{
    char date[13];
    int entry_start_line = 2;
    snprintf(date, 13, "%.2d:%.2d-%.2d:%.2d:", entry->start.tm_hour,
             entry->start.tm_min, entry->end.tm_hour, entry->end.tm_min);
    mvwprintw (win, entry_start_line+index, 0, "%s", date);
    mvwprintw (win, entry_start_line+index, strlen(date)+1, "%s", entry->header);

}


int ui_agenda_menu (struct settings *set, struct cal *cal)
{
    WINDOW *top;
    WINDOW *win;
    bool loop = true;
    struct tm *day = get_today();
    char select;

    clear();

    /* content window */
    top = newwin(1, COLS, 0, 0);
    win = newwin(LINES-3, COLS, 1, 0);

    if (set->color)
        wbkgd(win, A_NORMAL|COLOR_PAIR(CP_CONTENT));

    do {
        update_top_bar(top, set, "q:Quit  a:Add item  j:Next day  k:Previous day");
        werase(win);
        ui_show_day_agenda(win, day, set, cal);
        select = wgetch(win);

        switch (select) {
        case 'j':
            next_day(day);
            break;
        case 'k':
            prev_day(day);
            break;
        case 'q':
            loop = false;
            break;
        default:
            break;
        }
    } while (loop);

    free(day);
    delwin(top);
    delwin(win);

    return 1;
}


static int ui_show_day_agenda (WINDOW *win, const struct tm *day,  const struct settings *set,
                               const struct cal *cal)
{
    char * time_str;
    int winx, winy;
    struct vector *entries = cal->entries;
    struct entry *entry;
    int entries_day = 0;

    getmaxyx(win, winy, winx);
    werase(win);

    time_str = malloc(winy+1);
    strftime(time_str, winy+1, "%A %Y-%m-%d", day);

    mvwprintw(win, 1, 0, time_str);

    if (entries->elements > 0) {
        for(int i=0; i < entries->elements && winx > i; ++i) {
            entry = (struct entry *) vector_get(entries, i);
            if (entry_is_today(entry, day)) {
                print_agenda_day_entry(win, entry, entries_day++);
            }
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
    update_top_bar(NULL, set, "q:Quit");
    struct vector *entries = cal->entries;
    WINDOW *d_win = newwin(LINES-1, COLS, 1, 0);
    char select;
    bool exit = false;

    if (set->color)
        wbkgd(d_win, A_NORMAL|COLOR_PAIR(CP_CONTENT));

    for (int i = 0; i<entries->elements; ++i) {
        size_t size = 32;
        char start[size];
        char end[size];
        struct entry * tmp = vector_get(entries, i);
        int line = i+1;

        strftime(start, size, "%F %H:%M", &tmp->start);
        strftime(end, size, "%F %H:%M", &tmp->end);

        mvwprintw(d_win, line, 0, "%s -> %s: %s", start, end, tmp->header);
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


static void show_main_menu(WINDOW *win, struct settings *set, struct cal *cal)
{
    int line = 1;
    update_top_bar(NULL, set, "q:Quit  s:Save cal  d:Dump all entries");

    werase(win);
    mvwprintw(win, line++, 0, "a: show day agenda");
    mvwprintw(win, line++, 0, "A: add a new entry");

    wrefresh(win);
}


int ui_show_main_view (struct settings *set, struct cal *cal)
{
    bool exit = false;
    char select;
    WINDOW * main_win;
    struct tm *today = get_today();

    ui_init(set);

    main_win = newwin(LINES-1, COLS, 1, 0);

    if (set->color)
        wbkgd(main_win, A_NORMAL|COLOR_PAIR(CP_CONTENT));

    while (!exit) {
        werase(main_win);
        show_main_menu(main_win, set, cal);
        wrefresh(main_win);

        select = wgetch(main_win);
        switch (select) {
        case 'A':
            ui_add_entry(main_win, set, cal);
            break;
        case 'a':
            ui_agenda_menu(set, cal);
            break;
        case 'd':
            ui_show_dump(set, cal);
            break;
        case 'q':
            if (set->cal_changed && prompt_for_save(set))
                cal_save(cal, set->cal_file);
            exit = true;
            break;
        case 's':
            cal_save(cal, set->cal_file);
            break;
        default:
            break;
        }
    }

    free(today);
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
        wattron(win, COLOR_PAIR(CP_HEADER));
        for (int i = 0; i<COLS; ++i)
            mvwprintw(win, 0, i, " ");
    }
    mvwprintw(win, 0, 0, str);

    if (set->color)
        wattroff(win, COLOR_PAIR(CP_HEADER));

    wrefresh(win);

    if (m_all)
        delwin(win);
}

