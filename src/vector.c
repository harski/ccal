/* Copyright 2011-2012 Tuomo Hartikainen <hartitu@gmail.com>.
 * Licensed under the 2-clause BSD license, see LICENSE. */


#include <stdlib.h>
#include <string.h>

#include "vector.h"


static int vector_resize (struct vector *v, size_t size)
{
    if (size > 0) {
        void ** tmp = (void **) malloc(size*sizeof(void *));
        if (tmp==NULL)
            return 0;

        memcpy(tmp, v->list, v->elements*sizeof(void*));

        free(v->list);
        v->list = tmp;
        v->size = size;
    }

    return size;
}


int vector_add (struct vector *v, void *element)
{
    /* If list too small, expand */
    if(v->elements == v->size)
        if(!vector_resize(v, 2*v->size))
            return 0;

    v->list[v->elements] = element;
    v->elements += 1;

    return v->elements;
}


int vector_add_vector (struct vector *v1, const struct vector *v2)
{
    unsigned int i;
    for (i=0; i<v2->elements; ++i) {
        if (vector_add(v1, vector_get(v2, i)))
            return 0;
    }

    return v1->elements;
}


struct vector * vector_init ()
{
    return vector_init_size(VECTOR_DEFAULT_SIZE);
}


struct vector * vector_init_size (size_t size)
{
    struct vector *v = (struct vector *) malloc(sizeof(struct vector));

    if (v==NULL)
        return NULL;

    v->list = (void **) malloc(size*sizeof(void *));

    v->size = size;
    v->elements = 0;

    return v;
}


void vector_destroy (struct vector * v)
{
    free(v->list);
    free(v);
}


void * vector_get (const struct vector *v, unsigned int index)
{
    if (v->elements <= index)
        return NULL;

    return v->list[index];
}


void * vector_get_last (const struct vector *v)
{
    if (v->elements==0)
        return NULL;
    else
        return vector_get (v, v->elements - 1);
}


void * vector_remove (struct vector *v, unsigned int index)
{
    void *tmp;

    if (v->elements <= index)
        return NULL;

    tmp = v->list[index];

    memmove(v->list+index, v->list+index+1, (v->elements-index-1)*sizeof(void *));

    v->elements -= 1;

    return tmp;
}


void * vector_remove_last (struct vector *v)
{
    if (v->elements==0)
        return NULL;
    else
        return vector_remove(v, v->elements - 1);
}


void vector_compact (struct vector *v)
{
    if (v->size > v->elements) {
        if (v->elements==0)
            vector_resize(v, 1);
        else
            vector_resize(v, v->elements);
    }
}

