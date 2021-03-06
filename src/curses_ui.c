/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE

#include "appt.h"
#include "curses_ui.h"
#include "enter.h"
#include "log.h"
#include "strutils.h"
#include "timeframe.h"
#include "timeutils.h"
#include "todo.h"
#include "vector.h"
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


static void clear_all_wins(WINDOW **wins);
static void destroy_windows(WINDOW **wins);
static WINDOW **init_windows();
static int ui_add_appt (WINDOW **win, struct settings *set,
                  struct cal *cal);
static int ui_add_todo(WINDOW **wins, struct settings *set, struct cal *cal);
static void ui_init_color(const struct settings *set);
static int ui_show_day_agenda (WINDOW *win, struct tm *day,
                               const struct settings *set, const struct cal *cal);
static void ui_schedule_menu (WINDOW **wins, struct settings *set, struct vector *sch);
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


static void clear_all_wins(WINDOW **wins)
{
    for(int i=0; i<W_COUNT; ++i)
        wclear(wins[i]);
}


static void print_appt (WINDOW *win, struct appt *appt)
{
    const int start_col = 2;
    const int start_content = 15;
    const int start_line = 2;
    char str[128];

    mvwprintw(win, start_line+APPT_HEADER, start_col, "Header: ");
    if (appt->header != NULL)
        mvwprintw(win, start_line+APPT_HEADER, start_content, "%s", appt->header);

    mvwprintw(win, start_line+APPT_DESCRIPTION, start_col, "Description: ");
    if (appt->description != NULL)
        mvwprintw(win, start_line+APPT_DESCRIPTION, start_content, "%s", appt->description);

    mvwprintw(win, start_line+APPT_CATEGORY, start_col, "Category: ");
    if (appt->category != NULL)
        mvwprintw(win, start_line+APPT_CATEGORY, start_content, "%s", appt->category);

    mvwprintw(win, start_line+APPT_START_TIME, start_col, "Start time: ");
    if (appt->tf->start != NULL) {
        tmtostr(appt->tf->start, str, 128);
        mvwprintw(win, start_line+APPT_START_TIME, start_content, "%s", str);
    }

    mvwprintw(win, start_line+APPT_END_TIME, start_col, "End time: ");
    if (appt->tf->end != NULL) {
        tmtostr(appt->tf->end, str, 128);
        mvwprintw(win, start_line+APPT_END_TIME, start_content, "%s", str);
    }

    wrefresh(win);
}


static char * get_string(WINDOW *win, char *str, const char *prompt)
{
    /* if header is not already allocated, allocate a new stirng and
     * use that. If it is allocated, edit the existing one. */
    size_t size;
    char *tmp;
    int success;

    if (str!=NULL && str[0]!='\0') {
        size = strlen(str)+1;
        tmp = malloc(size);

        if (tmp==NULL)
            return str;

        strcpy(tmp, str);
    } else {
        size = 64;
        tmp = malloc(size);

        if (tmp==NULL)
            return str;

        tmp[0] = '\0';
    }

    success = ui_edit_string(win, 0, 0, prompt, &tmp, &size);

    if (success) {
        /* "Compact" the string */
        char *new_tmp = malloc(strlen(tmp) + 1);

        if (new_tmp == NULL)
            return str;

        strcpy(new_tmp, tmp);

        free(tmp);
        tmp = new_tmp;

        if (str!=NULL)
            free(str);

        str = tmp;
    }

    return str;
}


/* If oldtm is not allocated, it gets allocated, and its address gets
 * returned. If it is allocated, the new value is assigned to the it.
 * Returns the address of current struct tm, or NULL on error or cancel */
static struct tm * get_time(WINDOW *win, struct tm *oldtm, const char *prompt)
{
    struct tm *ret;
    struct tm *tm = malloc(sizeof(struct tm));

    if (tm==NULL) {
        /* If malloc failed */
        free(tm);
        do_log(LL_ERROR, "Malloc fail in %s:%s:%d", __FILE__,
               __func__, __LINE__);
        ret = NULL;
    } else if ((ui_get_date(win, 0, 0, prompt, tm))) {
        /* if got a valid value from ui_get_date */
        if (oldtm==NULL) {
            /* If oldtm wasn't before this function allocated */
            oldtm = tm;
            ret = tm;
        } else {
            /* oldtm was allocated before this function */
            *oldtm = *tm;
            ret = oldtm;
            free(tm);
        }
    } else {
        /* ui_get_date returned with error or cancel */
        free(tm);
        ret = oldtm;
    }

    return ret;
}


static int ui_add_appt (WINDOW **wins, struct settings *set,
                        struct cal *cal)
{
    struct appt * appt = appt_init();
    int appt_added;
    bool loop;
    bool saved = false;

    clear_all_wins(wins);
    update_top_bar(wins[W_TOP_BAR], set, "q:Return  h:Edit header  s:Edit start"
                                         " time  e:Edit end time  c:Edit category"
                                         " d:Edit category");
    print_appt(wins[W_CONTENT], appt);

    appt->header = get_string(wins[W_INPUT_BAR], appt->header, "Header");
    print_appt(wins[W_CONTENT], appt);

    appt->tf->start = get_time(wins[W_INPUT_BAR], appt->tf->start, "Start time");
    print_appt(wins[W_CONTENT], appt);

    appt->tf->end = get_time(wins[W_INPUT_BAR], appt->tf->end, "End time");

    /* Main loop */
    loop = true;
    while (loop) {
        /* Update appt information */
        wclear(wins[W_CONTENT]);
        print_appt(wins[W_CONTENT], appt);

        switch (wgetch(wins[W_INPUT_BAR])) {
        case 'c':
            appt->category = get_string(wins[W_INPUT_BAR], appt->category, "Category");
            break;

        case 'd':
            appt->description = get_string(wins[W_INPUT_BAR], appt->description, "Description");
            break;

        case 'e':
            appt->tf->end = get_time(wins[W_INPUT_BAR], appt->tf->end, "End time");
            break;

        case 'h':
            appt->header = get_string(wins[W_INPUT_BAR], appt->header, "Header");
            break;

        case 'q':
            if (!saved) {
                /* Check if appt is valid */
                if (appt_validate(appt)) {
                    /* Ask if entry is to be saved */
                    if (ui_get_yes_no(wins[W_INPUT_BAR], 0, 0, "Save this entry?", 'y')) {
                        vector_add(cal->appts, appt);
                        appts_sort(cal->appts);
                        set->cal_changed = true;
                        appt_added = 1;
                    } else {
                        appt_added = 0;
                        appt_destroy(appt);
                    }

                    loop = false;

                } else { /* Appt is not valid */
                    if(ui_get_yes_no(wins[W_INPUT_BAR], 0, 0, "Entry is invalid, discard?", 'n')) {
                        loop = false;
                        appt_destroy(appt);
                    } /* if we don't want to "discard", do nothing */
                }
            }
            break;

        case 's':
            appt->tf->start = get_time(wins[W_INPUT_BAR], appt->tf->start, "Start time");
            break;

        default:
            break;
        }
    }

    return appt_added;
}


static void print_todo (WINDOW *win, const struct todo *todo)
{
    const int start_col = 2;
    const int start_content = 15;
    const int start_line = 2;
    const size_t size = 32;
    char timestr[size];

    mvwprintw(win, start_line+TODO_HEADER, start_col, "Header: ");
    if (todo->header!=NULL)
        mvwprintw(win, start_line+TODO_HEADER, start_content, "%s", todo->header);

    mvwprintw(win, start_line+TODO_DESCRIPTION, start_col, "Description: ");
    if (todo->description!=NULL)
        mvwprintw(win, start_line+TODO_DESCRIPTION, start_content, "%s", todo->description);

    mvwprintw(win, start_line+TODO_CATEGORY, start_col, "Category: ");
    if (todo->category!=NULL)
        mvwprintw(win, start_line+TODO_CATEGORY, start_content, "%s", todo->category);

    mvwprintw(win, start_line+TODO_STATUS, start_col, "Status: ");
    mvwprintw(win, start_line+TODO_STATUS, start_content, "%s", todo_get_status_name(todo->status));

    mvwprintw(win, start_line+TODO_DEADLINE, start_col, "Deadline: ");
    if (todo->deadline!=NULL) {
        tmtostr(todo->deadline, timestr, size);
        mvwprintw(win, start_line+TODO_DEADLINE, start_content, "%s", timestr);
    }

    /* TODO: Add the scheduled times */

    wrefresh(win);
}


static int ui_add_todo(WINDOW **wins, struct settings *set, struct cal *cal)
{
    bool loop = true;
    int success = 1;
    struct todo *todo = todo_init();

    clear_all_wins(wins);
    update_top_bar(wins[W_TOP_BAR], set, "q:Return  h:Edit header");

    while (loop) {
        /* Update todo information */
        wclear(wins[W_CONTENT]);
        print_todo(wins[W_CONTENT], todo);

        switch (wgetch(wins[W_INPUT_BAR])) {
        case 'c':
            todo->category = get_string(wins[W_INPUT_BAR], todo->category, "Category");
            break;
        case 'd':
            todo->description = get_string(wins[W_INPUT_BAR], todo->description, "Description");
            break;
        case 'D':
            todo->deadline = get_time(wins[W_INPUT_BAR], todo->deadline, "Deadline");
            break;
        case 'h':
            todo->header = get_string(wins[W_INPUT_BAR], todo->header, "Header");
            break;
        case 'q':
            /* Check if todo is valid */
            if (todo_validate(todo)) {
                /* Ask if entry is to be saved */
                if (ui_get_yes_no(wins[W_INPUT_BAR], 0, 0, "Save this entry?", 'y')) {
                    vector_add(cal->todos, todo);
                    set->cal_changed = true;
                } else {
                    todo_destroy(todo);
                    success = 0;
                }

                loop = false;

            } else { /* Todo is not valid */
                if(ui_get_yes_no(wins[W_INPUT_BAR], 0, 0, "Entry is invalid, discard?", 'n')) {
                    loop = false;
                    todo_destroy(todo);
                    success = 0;
                }
            }
            break;
        case 's':
                ui_schedule_menu(wins, set, todo->scheduled);
            break;
        case 'S':
            todo->status = (todo->status + 1) % TS_COUNT;
            break;
        default:
            break;
        }
    }

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
    struct tm *day = get_today(NULL);
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


static int ui_show_day_agenda (WINDOW *win, struct tm *day,
                               const struct settings *set,
                               const struct cal *cal)
{
    char * time_str;
    int winx, winy;
    struct vector *appts_today;

    getmaxyx(win, winy, winx);
    werase(win);

    time_str = malloc(winy+1);
    strftime(time_str, winy+1, "%A %Y-%m-%d", day);

    mvwprintw(win, 1, 0, time_str);

    appts_today = appts_get_for_day(cal->appts, day);

    if (appts_today!=0)
        for (unsigned int i=0; i<appts_today->elements; ++i)
            print_agenda_day_appt(win, (struct appt *)vector_get(appts_today, i), i);
    else
        mvwprintw(win, 4, 3, "Nothing to do today :)");

    wrefresh(win);

    vector_destroy(appts_today);
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
    update_top_bar(wins[W_TOP_BAR], set, "q:Quit  s:Save cal  D:Dump all entries");

    werase(wins[W_CONTENT]);
    mvwprintw(wins[W_CONTENT], line++, 0, "d: show day agenda");
    mvwprintw(wins[W_CONTENT], line++, 0, "a: add a new appointment");
    mvwprintw(wins[W_CONTENT], line++, 0, "t: add a new todo");

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
    struct tm *today = get_today(NULL);

    WINDOW ** wins = ui_init(set);

    main_win = wins[W_CONTENT];

    while (!exit) {
        werase(main_win);
        show_main_menu(wins, set, cal);

        select = wgetch(wins[W_INPUT_BAR]);
        switch (select) {
        case 'a':
            ui_add_appt(wins, set, cal);
            break;
        case 'd':
            ui_agenda_menu(wins, set, cal);
            break;
        case 'D':
            ui_show_dump(wins, set, cal);
            break;
        case 'q':
            if (set->cal_changed && ui_get_yes_no(wins[W_INPUT_BAR], 0, 0, "Save changes to cal?", 'y')) {
                cal_save(cal, set->cal_file);
                set->cal_changed = false;
            }
            exit = true;
            break;
        case 's':
            if (set->cal_changed)
                cal_save(cal, set->cal_file);
            set->cal_changed = false;
            break;
        case 't':
            ui_add_todo(wins, set, cal);
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


static void ui_show_schedules (WINDOW *win, const struct vector *sch)
{
    const int col = 3;
    const int row = 2;
    char start[32];
    char end[32];
    struct timeframe *tf;
    unsigned int i;

    if (sch->elements==0) {
        mvwprintw(win, row, col, "No scheduled times");
    } else {
        for (i = 0; i<sch->elements; ++i) {
            tf = vector_get(sch, i);
            tmtostr(tf->start, start, 32);
            tmtostr(tf->end, end, 32);

            mvwprintw(win, row+i, col, "%s -> %s", start, end);
        }
    }

    wrefresh(win);
}


static int get_schedule_input (WINDOW *win, struct vector *sch)
{
    struct timeframe *tf = timeframe_init_alloc();
    int ret;

    ret = ui_get_date(win, 0, 0, "Start time", tf->start);
    if (ret) {
        ret = ui_get_date(win, 0, 0, "End time", tf->end);
        if (ret && timeframe_validate(tf))
            vector_add(sch, (void *)tf);
    }

    if (!ret)
        timeframe_destroy(tf);

    return ret;
}


static void ui_schedule_menu (WINDOW **wins, struct settings *set, struct vector *sch)
{
    bool exit = false;
    int select;

    clear_all_wins(wins);
    update_top_bar(wins[W_TOP_BAR], set, "q:Go back  a:Add scheduled time");

    while (!exit) {
        werase(wins[W_CONTENT]);
        ui_show_schedules(wins[W_CONTENT], sch);

        select = wgetch(wins[W_INPUT_BAR]);

        switch(select) {
        case 'a':
            get_schedule_input(wins[W_INPUT_BAR], sch);
            break;
        case 'q':
            exit = true;
            break;

        default:
            break;
        }
    }
}


static void update_top_bar (WINDOW * win, const struct settings *set,
                            const char *str)
{
    bool m_all = false;
    if (win==NULL) {
        win = newwin(1, COLS, 0, 0);
        if (win==NULL) {
            do_log(LL_ERROR, "%s" "Error in creating window for 'update_top_bar'");
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

