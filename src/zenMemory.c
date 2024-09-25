#include "zenVM.h"
#include <stdlib.h>
#include <stdio.h>

void *zmem_allocate(void *p, size_t o, size_t n) {
    vm.allocated += n - o;
    if (!n) {
        if (p) free(p);
        return NULL;
    }
    return realloc(p, n);
}