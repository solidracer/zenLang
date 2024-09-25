#ifndef MEM_H
#define MEM_H
#include <memory.h>
#include <string.h>
#include <stddef.h>

/* memory functions and needed values/types */

#define EPSILON 1e-9
typedef unsigned char byte;
typedef unsigned int len_t;
typedef unsigned long hash_t;
typedef unsigned int uint_t;
typedef unsigned short ushrt_t;

void *zmem_allocate(void *p, size_t o, size_t n);

#define ALLOC(t) zmem_allocate(NULL, 0, sizeof(t))
#define ALLOCS(siz) zmem_allocate(NULL, 0, siz)

#define FREE(ptr) zmem_allocate(ptr, sizeof(*ptr), 0)
#define FREES(ptr, siz) zmem_allocate(ptr, siz, 0)

#define REALLOC(ptr, oldn, nmem) zmem_allocate(ptr, (oldn)*sizeof(*ptr), (nmem)*sizeof(*ptr))

#endif