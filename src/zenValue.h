#ifndef VAL_H
#define VAL_H
#include "zenMemory.h"

typedef enum {
    TYPE_NUMBER,
    TYPE_NULL,
    TYPE_BOOL,
    TYPE_STRING,
} valuetype;

typedef struct {
    byte t;
    union {
        double n;
        int b;
        void *o;
    } val;
} value;

#define SETVAL(v, typ, m, to) ((v)->t = typ, (v)->val.m = to)
#define GETVAL(v, m) ((v)->val.m)

#endif