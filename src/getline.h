/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#ifndef GETLINE_H
#define GETLINE_H

#include <stddef.h>
#include <stdio.h>

int getline_custom (char **line, size_t *size, FILE *file);

#endif /* GETLINE_H */

