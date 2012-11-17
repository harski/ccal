/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE

#include "appt.h"
#include "config.h"
#include "curses_ui.h"
#include "enter.h"
#include "strutils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


enum ColorPairs {
    CP_HEADER = 1,
    CP_CONTENT
};


enum WindowNames {
    W_TOP_BAR,
    W_CONTENT,
    W_INFO_BAR,
    W_INPUT_BAR,
    W_COUNT
};


static bool appt_is_today(const struct appt *appt, const struct tm *tm);
static void clear_all_wins(WINDOW **wins);
static inline struct tm *get_today();
static void destroy_windows(WINDOW **wins);
static WINDOW **init_windows();
static inline void next_day (struct tm *tm);
static inline void prev_day (struct tm *tm);
static bool prompt_for_save (WINDOW *win, const struct settings *set);
static bool same_day (const struct tm *t1, const struct tm *t2);
static int ui_add_appt (WINDOW **win, struct settings *set,
                  struct cal *cal);
static void ui_init_color(const struct settings *set);
static int ui_show_day_agenda (WINDOW *win, const struct tm *day, const struct settings *set,
                               const struct cal *cal);
static void update_top_bar (WINDOW * win, const struct settings *set,
                            const char *str);



/* Initialize all the windows we are going to use.
 * This is only sensible if COLS > 3. */
/* TODO: ensure that terminal has enough LINES. Perhaps a COLS check too? */
static WINDOW **init_windows()
{
    WINDOW **wins = malloc(sizeof(WINDOW *) * W_COUNT);

    /* Top bar, info bar and input bar are all one line tall. The content
     * takes the rest */
    wins[W_TOP_BAR] = newwin(1, COLS, 0, 0);
    wins[W_CONTENT] = newwin(LINES-3, COLS, 1, 0);
    wins[W_INFO_BAR] = newwin(1, COLS, LINES-2, 0);
    wins[W_INPUT_BAR] = newwin(1, COLS, LINES-1, 0);

    return wins;
}


static void destroy_windows(WINDOW **wins)
{
    for (int i=W_TOP_BAR; i<W_COUNT; ++i)
        delwin(wins[i]);
}


static bool appt_is_today(const struct appt *appt, const struct tm *tm)
{
    if (same_day(appt->tf->start, tm) || same_day(appt->tf->end, tm))
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


static void clear_all_wins(WINDOW **wins)
{
    for(int i=0; i<W_COUNT; ++i)
        wclear(wins[i]);
}


bool prompt_for_save (WINDOW *win, const struct settings *set)
{
    int input;
    bool input_ok = false;
    bool ans;

    werase(win);

    mvwprintw(win, 0, 0, "Save changes to cal? [Y/n]: ");
    wrefresh(win);

    do {
        input = wgetch(win);
        if (input < 128) {
            if (input=='n') {
                input_ok = true;
                ans = false;
            } else if (input=='y' || input=='Y' || input=='\n') {
                input_ok = true;
                ans = true;
            }
        }
    } while (!input_ok);

    return ans;
}


static bool same_day (const struct tm *t1, const struct tm *t2)
{
    if (t1->tm_year == t2->tm_year && t1->tm_mon == t2->tm_mon &&
        t1->tm_mday == t2->tm_mday)
        return true;

    return false;
}


static void print_appt (WINDOW *win, struct appt *appt)
{
    const int start_col = 2;
    int line = 2;
    char str[128];

    mvwprintw(win, line++, start_col, "Header: ");
    if (appt->header != NULL)
        wprintw(win, "%s", appt->header);

    mvwprintw(win, line++, start_col, "Description: ");
    if (appt->description != NULL)
        wprintw(win, "%s", appt->description);

    mvwprintw(win, line++, start_col, "Category: ");
    if (appt->category != NULL)
        wprintw(win, "%s", appt->category);

    mvwprintw(win, line++, start_col, "Start time: ");
    if (appt->tf->start != NULL) {
        tmtostr(appt->tf->start, str, 128);
        wprintw(win, "%s", str);
    }

    mvwprintw(win, line++, start_col, "End time: ");
    if (appt->tf->end != NULL) {
        tmtostr(appt->tf->end, str, 128);
        wprintw(win, "%s", str);
    }

    wrefresh(win);
}


/* TODO: tss and tse are ugly, fix it */
static int ui_add_appt (WINDOW **wins, struct settings *set,
                        struct cal *cal)
{
    struct appt * appt = appt_init();
    size_t tmp_size = 128;
    char *tmp = malloc(tmp_size);
    int success = 1;
    int line = 2;
    bool date_ok;
    bool loop;
    bool saved = false;

    update_top_bar(wins[W_TOP_BAR], set, "q:Cancel  return:Select  s:Save");
    werase(wins[W_CONTENT]);
    wclear(wins[W_INFO_BAR]);
    wclear(wins[W_INPUT_BAR]);

    print_appt(wins[W_CONTENT], appt);

    ui_get_string(wins[W_INPUT_BAR], 0, 0, "Header", &tmp, &tmp_size);
    appt->header = malloc(1+strlen(tmp));
    strcpy(appt->header, tmp);
    tmp[0] = '\0';

    print_appt(wins[W_CONTENT], appt);

    date_ok = false;
    struct tm *tss = malloc(sizeof(struct tm));;
    while (!date_ok) {
        int date_ret = ui_get_date(wins[W_INPUT_BAR], 0, 0, "start time", tss);
        if (date_ret) {
            /* TODO: this needs to be _copied_ */
            appt->tf->start = tss;
            date_ok = true;
        }
        /* TODO: else: abort */
    }

    wclear(wins[W_INPUT_BAR]);
    print_appt(wins[W_CONTENT], appt);

    date_ok = false;
    struct tm *tse = malloc(sizeof(struct tm));;
    while (!date_ok) {
        int date_ret = ui_get_date(wins[W_INPUT_BAR], 0, 0, "end time", tse);
        if (date_ret) {
            /* TODO: this needs to be _copied_ */
            appt->tf->end = tse;
            date_ok = true;
        }
        /* TODO: else: abort */
    }

    wclear(wins[W_INPUT_BAR]);
    print_appt(wins[W_CONTENT], appt);

    wclear(wins[W_INPUT_BAR]);

    /* TODO: main loop */
    loop = true;
    while (loop) {
        print_appt(wins[W_CONTENT], appt);
        char select = wgetch(wins[W_INPUT_BAR]);

        switch (select) {
        case 'q':
            loop = false;
            if (!saved) {
                if(ui_get_yes_no(wins[W_INPUT_BAR], 0, 0, "Save this entry", 'y')) {
                    /* TODO: handle the saving, too! */
                    vector_add(cal->appts, appt);
                }
            }
            break;

        case 's':
            /* TODO: save it, too. I mean, why not? */
            saved = true;
            vector_add(cal->appts, appt);
            break;

        default:
            break;
        }

    }

    free(tmp);

    /* TODO: this value doesn't make sense anymore! */
    return success;
}


/* TODO: check if it is possible to set sensible themes for
 * non-color terminals. I.e. have ui_init_color() set also
 * attributes for no-color mode */
WINDOW ** ui_init (struct settings *set)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    WINDOW ** wins = init_windows();

    if (has_colors() && set->color) {
        ui_init_color(set);
        /* Assign colors to windows */
        wattron(wins[W_TOP_BAR], COLOR_PAIR(CP_HEADER));
        wbkgd(wins[W_TOP_BAR], COLOR_PAIR(CP_HEADER));
        wattron(wins[W_CONTENT], COLOR_PAIR(CP_CONTENT));
        wattron(wins[W_INFO_BAR], COLOR_PAIR(CP_HEADER));
        wbkgd(wins[W_INFO_BAR], COLOR_PAIR(CP_HEADER));
        wattron(wins[W_INPUT_BAR], COLOR_PAIR(CP_CONTENT));
        wbkgd(wins[W_INPUT_BAR], COLOR_PAIR(CP_CONTENT));
    }

    return wins;
}


static void ui_init_color(const struct settings *set) {
    start_color();
    init_pair(CP_HEADER, COLOR_YELLOW, COLOR_BLUE);
    init_pair(CP_CONTENT, COLOR_WHITE, COLOR_BLACK);
}


static inline void print_agenda_day_appt (WINDOW *win, const struct appt *appt, unsigned int index)
{
    char date[13];
    int appt_start_line = 2;
    snprintf(date, 13, "%.2d:%.2d-%.2d:%.2d:", appt->tf->start->tm_hour,
             appt->tf->start->tm_min, appt->tf->end->tm_hour, appt->tf->end->tm_min);
    mvwprintw (win, appt_start_line+index, 0, "%s", date);
    mvwprintw (win, appt_start_line+index, strlen(date)+1, "%s", appt->header);

}


int ui_agenda_menu (WINDOW **wins, struct settings *set, struct cal *cal)
{
    bool loop = true;
    struct tm *day = get_today();
    char select;

    clear_all_wins(wins);

    do {
        update_top_bar(wins[W_TOP_BAR], set, "q:Quit  a:Add item  j:Next day  k:Previous day");
        ui_show_day_agenda(wins[W_CONTENT], day, set, cal);
        select = wgetch(wins[W_INPUT_BAR]);

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

    return 1;
}


static int ui_show_day_agenda (WINDOW *win, const struct tm *day,
                               const struct settings *set, const struct cal *cal)
{
    char * time_str;
    int winx, winy;
    struct vector *appts = cal->appts;
    struct appt *appt;
    int appts_day = 0;

    getmaxyx(win, winy, winx);
    werase(win);

    time_str = malloc(winy+1);
    strftime(time_str, winy+1, "%A %Y-%m-%d", day);

    mvwprintw(win, 1, 0, time_str);

    if (appts->elements > 0) {
        for(unsigned int i=0; i < appts->elements && (unsigned)winx > i; ++i) {
            appt = (struct appt *) vector_get(appts, i);
            if (appt_is_today(appt, day)) {
                print_agenda_day_appt(win, appt, appts_day++);
            }
        }
    } else {
        mvwprintw(win, 4, 3, "Nothing to do today :)");
    }

    wrefresh(win);

    free(time_str);
    return 1;
}


int ui_show_dump (WINDOW **wins, struct settings *set, struct cal *cal)
{
    struct vector *appts = cal->appts;
    char select;
    bool exit = false;

    clear_all_wins(wins);
    update_top_bar(wins[W_TOP_BAR], set, "q:Quit");

    for (unsigned int i = 0; i<appts->elements; ++i) {
        size_t size = 32;
        char start[size];
        char end[size];
        struct appt * tmp = vector_get(appts, i);
        int line = i+1;

        strftime(start, size, "%F %H:%M", tmp->tf->start);
        strftime(end, size, "%F %H:%M", tmp->tf->end);

        mvwprintw(wins[W_CONTENT], line, 0, "%s -> %s: %s", start, end, tmp->header);
    }

    wmove(wins[W_CONTENT], 0, 0);
    wrefresh(wins[W_CONTENT]);

    while (!exit) {
        select = wgetch(wins[W_CONTENT]);
        switch (select) {

        case 'q':
            exit = true;
            break;

        default:
            break;
        }
        wrefresh(wins[W_CONTENT]);
    }

    return 1;
}


static void show_main_menu(WINDOW **wins, struct settings *set, struct cal *cal)
{
    int line = 1;
    update_top_bar(wins[W_TOP_BAR], set, "q:Quit  s:Save cal  d:Dump all appts");

    werase(wins[W_CONTENT]);
    mvwprintw(wins[W_CONTENT], line++, 0, "a: show day agenda");
    mvwprintw(wins[W_CONTENT], line++, 0, "A: add a new appt");

    wrefresh(wins[W_CONTENT]);

    wmove(wins[W_INFO_BAR], 0, 0);
    wclrtoeol(wins[W_INFO_BAR]);
    wrefresh(wins[W_INFO_BAR]);

    wmove(wins[W_INPUT_BAR], 0, 0);
    wclrtoeol(wins[W_INPUT_BAR]);
    wrefresh(wins[W_INPUT_BAR]);
}


int ui_show_main_view (struct settings *set, struct cal *cal)
{
    bool exit = false;
    char select;
    WINDOW * main_win;
    struct tm *today = get_today();

    WINDOW ** wins = ui_init(set);

    main_win = wins[W_CONTENT];

    while (!exit) {
        werase(main_win);
        show_main_menu(wins, set, cal);

        select = wgetch(wins[W_INPUT_BAR]);
        switch (select) {
        case 'A':
            ui_add_appt(wins, set, cal);
            break;
        case 'a':
            ui_agenda_menu(wins, set, cal);
            break;
        case 'd':
            ui_show_dump(wins, set, cal);
            break;
        case 'q':
            if (set->cal_changed && prompt_for_save(wins[W_INPUT_BAR], set)) {
                cal_save(cal, set->cal_file);
                set->cal_changed = false;
            }
            exit = true;
            break;
        case 's':
            cal_save(cal, set->cal_file);
            set->cal_changed = false;
            break;
        default:
            break;
        }
    }

    free(today);
    destroy_windows(wins);
    endwin();

    return 1;
}


static void update_top_bar (WINDOW * win, const struct settings *set,
                            const char *str)
{
    bool m_all = false;
    if (win==NULL) {
        win = newwin(1, COLS, 0, 0);
        if (win==NULL) {
            fprintf(stderr, "Error in creating window for 'update_top_bar'\n");
            return;
        }

        /* Set the correct color theme. No need to unset this below,
         * as the window gets deleted */
        if (set->color)
            wattron(win, COLOR_PAIR(CP_HEADER));
        m_all = true;
    }

    mvwprintw(win, 0, 0, str);
    wclrtoeol(win);
    wrefresh(win);

    if (m_all)
        delwin(win);
}

