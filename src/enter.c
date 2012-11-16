/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE_EXTENDED

#include "config.h"
#include "enter.h"
#include "log.h"
#include "strutils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vector.h>
#include <wchar.h>

#define CTRL(c) ((c) & 037)
#define STR_INIT_SIZE 16
#define PROMPT_CHAR ':'


static int wget_wchar (WINDOW *win, wchar_t *c)
{
    int get_ret;
    wint_t wic;

    get_ret = wget_wch(win, &wic);
    if (get_ret==OK && wic!=WEOF)
        *c = (wchar_t) wic;

    switch(get_ret) {
    case OK:
        get_ret = 0;
        break;
    case KEY_CODE_YES:
        get_ret = 1;
        break;
    default:
        get_ret = 2;
    }

    return get_ret;
}


static void tokenize (const char *str, struct vector *v)
{
    bool token_open = false;
    int token_len = 0;
    int token_start = 0;

    for (int i=0; (size_t)i <= strlen(str); ++i) {
        bool iws = (is_whitespace(str[i]) || str[i]=='\0');
        if (iws && token_open) {
            /* token ends */
            char *token = malloc(token_len+1);
            strncpy(token, str+token_start, token_len);
            token[token_len] = '\0';
            vector_add(v, token);
            token_len = 0;
            token_open = false;
        } else if (!iws && token_open) {
            ++token_len;
        } else if (iws && !token_open) {
            continue;
        } else if (!iws && !token_open) {
            token_open = true;
            token_start = i;
            ++token_len;
        }
    }
}


/* TODO: separate %m-%d from %H:%M: strlen is the same! */
static bool _do_match (const struct vector *v, struct tm *tm)
{
    bool year;
    bool month;
    bool day;
    bool hour;
    bool min;
    bool match;
    time_t tnow;
    struct tm tmnow;

    year = month = day = hour = min = match = false;

    tnow = time(NULL);
    localtime_r(&tnow, &tmnow);

    for (unsigned int i=0; i<v->elements; ++i) {
        const char *tmp = vector_get(v, i);
        struct tm tmtmp;
        int tmplen = strlen(tmp);

        if (tmplen==4) {
            if (strptime(tmp, "%H%M", &tmtmp) != NULL) {
                hour = min = true;
                tm->tm_hour = tmtmp.tm_hour;
                tm->tm_min = tmtmp.tm_min;
            } else if (strptime(tmp, "%H:%M", &tmtmp) != NULL) {
                hour = min = true;
                tm->tm_hour = tmtmp.tm_hour;
                tm->tm_min = tmtmp.tm_min;
            }
        } else {
            if (strptime(tmp, "%Y-%m-%d", &tmtmp) != NULL) {
                year = month = day = true;
                tm->tm_year = tmtmp.tm_year;
                tm->tm_mon = tmtmp.tm_mon;
                tm->tm_mday = tmtmp.tm_mday;
            } else if (strptime(tmp, "%m-%d", &tmtmp) != NULL) {
                year = false;
                month = day = true;

                if (tmtmp.tm_mon < tmnow.tm_mon || tmtmp.tm_mday < tmnow.tm_mday) {
                    tm->tm_year = tmnow.tm_year+1;
                } else {
                    tm->tm_year = tmnow.tm_year;
                }
                tm->tm_mon = tmtmp.tm_mon;
                tm->tm_mday = tmtmp.tm_mday;
            } else if (strptime(tmp, "%d", &tmtmp) != NULL) {
                year = month = false;
                day = true;

                tm->tm_year = tmnow.tm_year;
                tm->tm_mon = tmnow.tm_mon;
                tm->tm_mday = tmtmp.tm_mday;

                if (tmtmp.tm_mday < tmnow.tm_mday) {
                    /* increase month */
                    tm->tm_mon +=1;
                    if (tm->tm_mon==12) {
                        tm->tm_year+=1;
                        tm->tm_mon=0;
                    }
                }
            }
        }
    }

    if (!day && hour) {
        if (tm->tm_hour < tmnow.tm_hour ||
            (tm->tm_hour == tmnow.tm_hour && tm->tm_min < tmnow.tm_min)) {
            tm->tm_mday += 1;
            /* TODO: fetch the next day */
        } else {
            tm->tm_mday = tmnow.tm_mday;
        }
        match = true;
    } else if (day && !hour) {
        tm->tm_hour = 0;
        tm->tm_min = 0;
        match = true;
    } else if (day && hour) {
        match = true;
    } else {
        match = false;
    }

    return match;
}


static int match_date (const char *str, struct tm *tm)
{
    struct vector *v = vector_init();
    int valid;

    tokenize(str, v);

    /* match the tokens */;
    valid = _do_match(v, tm);

    while (v->elements > 0)
        free(vector_remove(v, 0));

    vector_destroy(v);

    return valid;
}


{
}


int ui_get_date (WINDOW *win, const int row, const int col,
                 const char *prompt, struct tm *tm)
{
    size_t tmp_size = 128;
    int tmp_len = 0;
    char * tmp = malloc(tmp_size);
    wchar_t wc;
    int get_ret;
    bool read = true;
    int return_val = 1;
    int date_ret = 0;
    int characters = 0;
    char time_str[128];
    int prompt_len = strlen(prompt);

    tm->tm_hour = 0;
    tm->tm_min = 0;

    wmove(win, row, col);
    if (prompt!=NULL)
        wprintw(win, "%s%c ", prompt, PROMPT_CHAR);

    curs_set(1);

    while (read) {
        get_ret = wget_wchar(win, &wc);
        if (!get_ret) {
            if (wc==CTRL('D') || wc=='\n') {
                if (date_ret)
                    read = false;
                continue;
            } else if (wc==127 || wc==KEY_DC || wc==KEY_BACKSPACE) {
                if (characters==0)
                    continue;

                --characters;

                /* TODO: Fix handling multi-byte characters:
                 * more than one char may need to be deleted */
                tmp[--tmp_len] = '\0';
            } else {
                /* normal character */
                int written;

                ++characters;
                written = wctomb(tmp+tmp_len, wc);
                if (written == -1) {
                    fprintf(stderr, "Can't transform wide char to multibyte in %s:%d:%s\n",
                           __FILE__, __LINE__, __func__);
                }

                tmp_len += written;
                tmp[tmp_len] = '\0';
            }
        } else {
            /* TODO: An error of sorts. Deal with it */
        }

        date_ret = match_date(tmp, tm);

        if (date_ret) {
            mktime(tm);
            tmtostr(tm, time_str, 128);
        } else {
            strncpy(time_str, "(Invalid date)", 128);
        }
        mvwprintw(win, row, col, "%s%c %s  => %s", prompt, PROMPT_CHAR, tmp, time_str);
        wclrtoeol(win);
        wmove(win, row, prompt_len + 2 + string_length(tmp));
        wrefresh(win);
    }

    curs_set(0);
    wrefresh(win);
    free(tmp);

    return return_val;
}


int ui_get_string (WINDOW *win, const int row, const int col,
                   const char *prompt, char **str, size_t *size)
{
    char *tmp;
    wchar_t wc;
    size_t tmp_len = 0;
    int get_ret;
    int written;
    int characters = 0;

    if (str==NULL) {
        *size = STR_INIT_SIZE;
        tmp = malloc(STR_INIT_SIZE);
        str = &tmp;
    } else {
        tmp = *str;
    }

    wmove(win, row, col);
    if (prompt!=NULL)
        wprintw(win, "%s%c ", prompt, PROMPT_CHAR);

    curs_set(1);

    while (true) {
        get_ret = wget_wchar(win, &wc);

        if (!get_ret) {
            if (wc==CTRL('D') || wc=='\n') {
                break;
            } else if (wc==127 || wc==KEY_DC || wc==KEY_BACKSPACE) {
                if (characters==0)
                    continue;

                --characters;

                /* TODO: Fix handling multi-byte characters:
                 * more than one char may need to be deleted */
                tmp[--tmp_len] = '\0';
            } else {
                /* normal character */
                ++characters;

                /* Ensure tmp has at least 5 bytes (utf-char + NUL) left! */
                if (*size - tmp_len < MB_CUR_MAX) {
                    *size *= 2;
                    tmp = realloc(tmp, *size);
                    if (tmp==NULL) {
                        fprintf(stderr, "Failed to allocate memory\n");
                        return 0;
                    }
                    str = &tmp;
                }

                written = wctomb(tmp+tmp_len, wc);
                if (written == -1)
                    wlog(NULL, "Can't transform wide char to multibyte in %s:%d:%s\n",
                           __FILE__, __LINE__, __func__);

                tmp_len += written;
                tmp[tmp_len] = '\0';
            }
        } else {
            /* TODO: An error of sorts. Deal with it */
        }

        /* Print the current situation to screen */
        mvwprintw(win, row, col, "%s%c %s", prompt, PROMPT_CHAR, tmp);
        wclrtoeol(win);
        wrefresh(win);
    }

    curs_set(0);
    wrefresh(win);

    return 1;
}

