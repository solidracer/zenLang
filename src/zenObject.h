#ifndef ZOBJ_H
#define ZOBJ_H
#include "zenValue.h"

typedef struct object {
    valuetype t;
    struct object *next;
} object;

object *zobj_allocate(size_t s, valuetype t);
void zobj_free(object *o);

#endif