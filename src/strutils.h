/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef STRUTILS_H
#define STRUTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>


/* Check if char is utf8 contiuation character */
bool is_utf8_cont_byte (char c);

/* Check if string contains only numeric values */
bool is_numeric (const char *str);

/* Check if char is a whitespace character */
bool is_whitespace (char c);

/* Clean whitespace from end/beginning */
size_t remove_postfix_whitespace (char *str, size_t len);
size_t remove_prefix_whitespace (char *str, size_t len);

/* If str is enclosed in quotes (with or without whitespace on the
 * "outer" side, remove the quotes */
void removequotes (char *str);

/* Split string to a pair if it has the separator */
int str_to_key_value_pairs (const char *str, const char separator, char *key,
                            size_t keylen, char *value, size_t valuelen);

/* Wide char string length in columns (not bytes) */
size_t string_length (const char *str);

/* Strip whitespace from beginning and end */
size_t strip (char *str, size_t len);

/* Traslate tm to string */
char * tmtostr (const struct tm *tm, char *str, const size_t size);

/* How many bytes the next character in array uses */
size_t utf8_char_size(const char *c);

#endif /* STRUTILS_H */

