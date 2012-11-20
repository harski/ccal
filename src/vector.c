/* Copyright 2011-2012 Tuomo Hartikainen <hartitu@gmail.com>.
 * Licensed under the 2-clause BSD license, see LICENSE. */


#include <stdlib.h>
#include <string.h>

#include "vector.h"


static int vector_resize (struct vector *v, size_t size)
{
    if (size > 0) {
        void * tmp = malloc(size*sizeof(void *));
        if (tmp==NULL) return 0;

        memcpy(tmp, v->list, v->elements*sizeof(void*));
        free(v->list);
        v->list = tmp;
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

    v->list = malloc(size*sizeof(void *));

    v->size = size;
    v->elements = 0;

    return v;
}


struct vector * vector_create ()
{
    return vector_init_size(VECTOR_DEFAULT_SIZE);
}


struct vector * vector_create_size (size_t size)
{
    return vector_init_size(size);
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


void * vector_remove (struct vector *v, unsigned int index)
{
    void *tmp;
    unsigned int i;

    if (v->elements <= index)
        return NULL;

    tmp = v->list[index];

    /* TODO: more efficent with memmove */
    for (i = index; i < v->elements-1; ++i)
        v->list[i] = v->list[i+1];

    v->elements -= 1;

    return tmp;
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

