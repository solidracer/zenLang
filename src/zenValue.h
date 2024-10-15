#ifndef ZVAL_H
#define ZVAL_H
#include "zenMemory.h"

typedef enum {
    TYPE_NUMBER,
    TYPE_BOOL,
    TYPE_NULL,
    TYPE_STRING,
    TYPE_TABLE,
    TYPE_NONE
} valuetype;
#define ZEN_TYPECOUNT (TYPE_NONE)

extern const char *zen_typenames[ZEN_TYPECOUNT];

typedef struct {
    valuetype t;
    union {
        double n;
        int b;
        void *o;
    } val;
} value;

#define GETVAL(v,n) ((v)->val.n)
#define SETVAL(v,ty,n,nv) ((v)->t = ty, (v)->val.n = nv)

#endif