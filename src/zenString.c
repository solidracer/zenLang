#include "zenVM.h"

static hash_t gethash(char *s, len_t len) {
    hash_t h = 5381;
    len_t i;
    for (i = 0;i<len;i++,s++) {
        h = ((h << 5) + h) + *s;
    }
    return h;
}

string *zstr_new(char *b, len_t len) {
    value v;
    hash_t h = gethash(b, len);
    string *interned, *s;
    if ((interned = ztab_xsearch(&vm.strings, b, len, h))) {
        return interned;
    }
    s = (string*)zobj_allocate(sizeof(string), TYPE_STRING);
    s->len = len;
    s->hash = h;
    s->str = ALLOCS(len+1);
    memcpy(s->str, b, len);
    s->str[len] = '\0';
    ztab_insert(&vm.strings, s, &v);
    return s;
}

string *zstr_take(char *b, len_t len) {
    value v;
    hash_t h = gethash(b, len);
    string *interned, *s;
    if ((interned = ztab_xsearch(&vm.strings, b, len, h))) {
        FREES(b, len+1);
        return interned;
    }
    s = (string*)zobj_allocate(sizeof(string), TYPE_STRING);
    s->len = len;
    s->hash = h;
    s->str = b;
    ztab_insert(&vm.strings, s, &v);
    return s;
}