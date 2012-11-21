/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#define _XOPEN_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "strutils.h"

#define WS "\t\n "

bool is_numeric (const char *str)
{
    for (int i=0; str[i]!='\0'; ++i)
        if (str[i]<48 || str[i]>57)
            return false;

    return true;
}


bool is_whitespace (char c)
{
    unsigned int i=0;
    while (WS[i++]!='\0')
        if (c==WS[i])
            return true;

    return false;
}


size_t remove_postfix_whitespace (char *str, size_t len)
{
    int i = len-1;
    unsigned int ws = 0;

    while (i>=0 && is_whitespace(str[i])) {
        --i;
        ++ws;
    }

    str[len-ws] = '\0';

    return len-ws;
}


size_t remove_prefix_whitespace (char *str, size_t len)
{
    unsigned int ws = 0;

    while (ws<len && is_whitespace(str[ws]))
        ++ws;

    if (ws!=0) {
        memmove(str, str+ws, len-ws+1);
        len -= ws;
    }
    return len;
}


void removequotes (char *str)
{
    if (str[0] == '"') {
        size_t len = strlen(str);
        if (str[len-1] == '"') {
            memmove(str, str+1, len-2);
            str[len-2] = '\0';
        }
    }
}


/* TODO: Get lengths as pointers, so the new lengths can be returned.
 * Their size is now only implied: min(keylen, strlen(key)) */
/* If key or value are NULL, they get allocated by realloc */
int str_to_key_value_pairs (const char *str, const char separator, char *key,
                            size_t keylen, char *value, size_t valuelen)
{
    int separator_index = -1;
    int str_len = strlen(str);

    for (int i=0; str_len; ++i) {
        if (str[i] == separator) {
            separator_index = i;
            break;
        }
    }

    if (separator_index == -1)
        return 0;

    /* Ensure that key fits */
    if (keylen < separator_index+1) {
        if(realloc(key, separator_index+1)==NULL) {
            /* Realloc failed. We should too */
            return 0;
        }
    }

    /* Ensure that value fits */
    if (valuelen < str_len-separator_index) {
        if(realloc(key, str_len-separator_index)==NULL) {
            return 0;
        }
    }

    strncpy(key, str, separator_index);
    key[separator_index] = '\0';

    strncpy(value, str+separator_index+1, strlen(str+separator_index+1));
    value[strlen(str+separator_index+1)] = '\0';

    return 1;
}


size_t string_length (const char *str)
{
    size_t len;
    wchar_t *wch;

    if (!str || str[0]=='\0')
        return 0;

    len = strlen(str) + 1;
    wch = malloc(len*sizeof(wchar_t));
    len = mbstowcs(wch, str, len);
    len = wcswidth(wch, len);
    free(wch);

    return len;
}


size_t strip (char *str, size_t len)
{
    len = remove_postfix_whitespace(str, len);
    return remove_postfix_whitespace(str, len);
}


char * tmtostr (const struct tm *tm, char *str, const size_t size)
{
    strftime(str, size, "%A %Y-%m-%d %H:%M", tm);
    return str;
}

