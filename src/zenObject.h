#ifndef OBJ_H
#define OBJ_H
#include "zenValue.h"

typedef struct obj {
    valuetype t;
    struct obj *next;
} object;

object *zobj_allocate(size_t size, valuetype objt);
void zobj_free(object *o);

#endif