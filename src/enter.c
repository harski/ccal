/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE_EXTENDED

#include "enter.h"
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#define CTRL(c) ((c) & 037)
#define STR_INIT_SIZE 16
#define PROMPT_CHAR ':'


/* TODO: echo inputted characters to screen */
int ui_get_string (WINDOW *win, const int row, const int col,
                   const char *prompt, char **str, size_t *size)
{
    char *tmp;
    wint_t wic;
    wchar_t wc;
    size_t tmp_len = 0;
    int get_ret;
    bool read = true;
    int written;

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

    do {
        get_ret = wget_wch(win, &wic);

        if (get_ret==OK) {
            if (wic==WEOF) {
                /* WTF just happened? */
                fprintf(stderr, "You just gave me a WEOF?!");
                return 0;
            } else {
                if (wc==CTRL('d')) {
                    read = false;
                    continue;
                }
                /* normal character */

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

                wc = (wchar_t) wic;
                written = wctomb(tmp+tmp_len, wc);

                if (written == -1) {
                    fprintf(stderr, "Can't transform wide char to multibyte in %s:%d:%s\n",
                           __FILE__, __LINE__, __func__);
                }

                tmp_len += written;
                tmp[tmp_len] = '\0';
            }
        } else if (get_ret==KEY_CODE_YES) {
            /* TODO: check for control characters! */
        } else {
            /* get_ret == ERR */
            fprintf(stderr, "wget_wch error in %s:%d:%s\n",
                    __FILE__, __LINE__, __func__);
        }
    } while (read);

    wrefresh(win);

    return 1;
}

