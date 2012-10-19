#include "config.h"
#include "curses_ui.h"
#include "entry.h"
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>


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


static void print_main_header (WINDOW * win, struct settings *set)
{
    if (set->color)
        attron(COLOR_PAIR(1));

    mvwprintw(win, 0, 0, "q:Quit  d:Dump entries");

    if (set->color)
        attroff(COLOR_PAIR(1));

    wrefresh(win);
}


int ui_show_main_view (struct settings *set, struct cal *cal)
{
    int row, col;
    bool exit = false;
    char dim_str[20];
    char select;
    int cur_line;
    WINDOW * main_win;

    ui_init(set);

    print_main_header(stdscr, set);

    main_win = newwin(LINES-1, COLS, 1, 0);

    getmaxyx(stdscr, row, col);
    snprintf(dim_str, 20, "%d x %d", row, col);

    mvwprintw(main_win, 0, col-1-strlen(dim_str), "%s", dim_str);

    cur_line = 2;
    mvwprintw(main_win, cur_line++, 1, "d: view all entries (dump)");

    ++cur_line;
    mvwprintw(main_win, cur_line++, 1, "q: quit ccal");
    wmove(main_win, 0, 0);

    wrefresh(main_win);

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

