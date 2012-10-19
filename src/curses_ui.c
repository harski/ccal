#include "curses_ui.h"
#include "entry.h"
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>


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
    int row, col;
    bool exit = false;
    char dim_str[20];
    char select;
    int cur_line;

    /* ncurses init stuff */
    initscr();
    cbreak();
    noecho();

    getmaxyx(stdscr, row, col);
    snprintf(dim_str, 20, "%d x %d", row, col);

    mvprintw(0, col-1-strlen(dim_str), "%s", dim_str);

    cur_line = 2;
    mvprintw(cur_line++, 1, "d: view all entries (dump)");

    ++cur_line;
    mvprintw(cur_line++, 1, "q: quit ccal");
    move(0, 0);

    refresh();

    while (!exit) {
        select = getch();
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
        refresh();
    }

    endwin();
    return 1;
}

