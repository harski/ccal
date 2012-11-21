/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef STRUTILS_H
#define STRUTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

bool is_numeric (const char *str);
bool is_whitespace (char c);
size_t remove_postfix_whitespace (char *str, size_t len);
size_t remove_prefix_whitespace (char *str, size_t len);
void removequotes (char *str);
int str_to_key_value_pairs (const char *str, const char separator, char *key,
                            size_t keylen, char *value, size_t valuelen);
size_t string_length (const char *str);
size_t strip (char *str, size_t len);
char * tmtostr (const struct tm *tm, char *str, const size_t size);

#endif /* STRUTILS_H */

