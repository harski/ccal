/* Copyright (C) 2012 Tuomo Hartikainen <hartitu@gmail.com>
 * Licensed under GPLv3, see LICENSE for more information. */

#include <stdlib.h>
#include "timeframe.h"


struct timeframe * timeframe_init()
{
    struct timeframe *tf = malloc(sizeof(tf));
    if (tf!=NULL) {
        tf->start = NULL;
        tf->end = NULL;
    }

    return tf;
}


struct timeframe * timeframe_init_alloc()
{
    struct timeframe *tf = timeframe_init();

    if (tf!=NULL) {
        tf->start = malloc(sizeof(struct tm));
        tf->end = malloc(sizeof(struct tm));

        if (tf->start==NULL || tf->end==NULL) {
            timeframe_destroy(tf);
            tf=NULL;
        }
    }

    return tf;
}


void timeframe_destroy (struct timeframe *tf)
{
    free(tf->start);
    free(tf->end);
    free(tf);
}

