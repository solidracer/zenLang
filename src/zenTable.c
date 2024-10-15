#include "zenTable.h"

static entry *findentry(table *t, string *s) {
    len_t h = s->hash & (t->cap-1);
    entry *tomb = NULL;
    for (;;) {
        entry *e = t->hash + h;
        if (!e->key) {
            if (GETVAL(&e->v, n) && !tomb)
                tomb = e;
            else return tomb?tomb:e;
        }
        else if (e->key == s) return e;
        h = (h+1) & (t->cap-1);
    }
}

void ztab_init(table *t) {
    t->hash = NULL;
    t->size = t->cap = 0;
}

void ztab_free(table *t) {
    REALLOC(t->hash, t->cap, 0);
    ztab_init(t);
}

void ztab_insert(table *t, string *key, value *v) {
    entry *e;
    if (t->size + 1 > t->cap*.75) {
        len_t oldcap = t->cap;
        len_t i;
        entry *oldhash = t->hash;
        t->cap = t->cap<8?8:t->cap*2;
        t->hash = ALLOCS(sizeof(entry)*t->cap);
        memset(t->hash, 0, sizeof(entry)*t->cap);
        if (oldhash) {
            for (i = 0;i<oldcap;i++) {
                entry *e = oldhash + i;
                entry *new;
                if (!e->key) continue;
                new = findentry(t, e->key);
                new->key = e->key;
                new->v = e->v;
            }
            REALLOC(oldhash, oldcap, 0);
        }
    }
    e = findentry(t, key);
    if (!e->key) t->size++;
    e->key = key;
    e->v = *v;
}

int ztab_search(table *t, string *key, value *v) {
    entry *e;
    if (!t->size) return ZEN_FAILURE;
    e = findentry(t, key);
    if (e->key!=key) return ZEN_FAILURE;
    *v = e->v;
    return ZEN_SUCCESS;
}

int ztab_set(table *t, string *key, value *v) {
    entry *e;
    if (!t->size) return ZEN_FAILURE;
    e = findentry(t, key);
    if (e->key!=key) return ZEN_FAILURE;
    e->v = *v;
    return ZEN_SUCCESS;
}

int ztab_remove(table *t, string *key) {
    entry *e;
    if (!t->size) return ZEN_FAILURE;
    e = findentry(t, key);
    if (e->key!=key) return ZEN_FAILURE;
    e->key = NULL; /* forgot to do this here */
    SETVAL(&e->v, 0, n, 1);
    t->size--;
    return ZEN_SUCCESS;
}

string *ztab_xsearch(table *t, char *s, len_t len, hash_t h) {
    len_t hash;
    if (!t->size) return NULL;
    hash = h & (t->cap-1);
    for (;;) {
        entry *e = t->hash + hash;
        if (e->key) {
            string *str = e->key;
            if (str->len == len && str->hash == h && !memcmp(str->str, s, len)) return str;
        }else return NULL;
        hash = (hash+1) & (t->cap-1);
    }
}

otable *ztab_alocnew() {
    otable *t = (otable*)zobj_allocate(sizeof(otable), TYPE_TABLE);
    ztab_init(GETTABLE(t));
    return t;
}