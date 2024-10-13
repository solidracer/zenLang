#include "zenVMext.h"
#include <stdlib.h>

void *zmem_allocate(void *p, size_t old, size_t new) {
    void *np;
    vm.allocated += new - old;
    if (!new) {
        if (p) free(p);
        return NULL;
    }
    np = realloc(p, new);
    if (!np) {
        perror("error");
        exit(ZEN_FAILURE);
    }
    return np;
}