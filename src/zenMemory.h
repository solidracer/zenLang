#ifndef ZMEM_H
#define ZMEM_H
#include "zenDef.h"
#include <string.h>

void *zmem_allocate(void *p, size_t old, size_t new);

#define ALLOC(t) zmem_allocate(NULL, 0, sizeof(t))
#define ALLOCS(siz) zmem_allocate(NULL, 0, siz)

#define FREE(ptr) zmem_allocate(ptr, sizeof(*ptr), 0)
#define FREES(ptr, siz) zmem_allocate(ptr, siz, 0)
#define REALLOC(ptr, old, new) zmem_allocate(ptr, (old)*sizeof(*(ptr)), (new)*sizeof(*(ptr)))

#endif