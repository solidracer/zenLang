#include "zenVM.h"

hash_t zstr_hash(char *s, len_t len) {
    hash_t h = 5381;
    for (len_t i = 0;i<len;i++) {
        char c = s[i];
        h = ((h << 5) + h) + c; /* h * 32 + h + c = h * 33 + c */
    }
    return h;
}

string *zstr_new(char *buf, len_t len) {
    string *interned;
    hash_t h = zstr_hash(buf, len);
    if ((interned = zmap_xsearch(&vm.strings, buf, len, h))) return interned;
    string *s = (string*)zobj_allocate(sizeof(string)+len+1, TYPE_STRING);
    s->hash = h;
    s->len = len;
    memcpy(s->str, buf, len);
    s->str[len] = '\0';
    zmap_insert(&vm.strings, s, &(value){TYPE_NUMBER, {.n = 0}});
    return s;
}
