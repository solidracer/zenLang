#include "zenVM.h"

object *zobj_allocate(size_t s, valuetype t) {
    object *o = ALLOCS(s);
    o->t = t;
    o->next = vm.objects;
    vm.objects = o;
    return o;
}

void zobj_free(object *o) {
    switch(o->t) {
        case TYPE_STRING: {
            string *s = (string*)o;
            FREES(s->str, s->len+1);
            FREE(s);
            break;
        }
        case TYPE_TABLE: {
            otable *t = (otable*)o;
            ztab_free(GETTABLE(t));
            FREE(t);
            break;
        }
        default: break;
    } 
}