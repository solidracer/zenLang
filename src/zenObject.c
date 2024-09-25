#include "zenVM.h"

object *zobj_allocate(size_t size, valuetype objt) {
    object *o = ALLOCS(size);
    o->t = objt;
    o->next = vm.objects;
    vm.objects = o;
    return o;
}

void zobj_free(object *o) {
    switch(o->t) {
        case TYPE_STRING: {
            string *s = (string*)o;
            FREES(s, sizeof(string)+s->len+1);
            break;
        }
        default: break;
    }
}