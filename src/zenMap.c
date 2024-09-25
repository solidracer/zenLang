#include "zenMap.h"

void zmap_init(map *m) {
    m->cap = m->size = 0;
    m->hash = NULL;
}

void zmap_free(map *m) {
    REALLOC(m->hash, m->cap, 0);
    zmap_init(m);
}

static entry *findentry(map *m, string *key) {
    len_t h = key->hash & (m->cap - 1);
    entry *tomb = NULL;
    for (;;) {
        entry *e = m->hash + h;
        if (!e->key) {
            if (GETVAL(&e->v, n) == DUMMYVALUE && !tomb) {
                tomb = e;
                continue;
            }
            return e;
        }else if (e->key == key) return tomb?tomb:e;
        h = (h + 1) & (m->cap - 1);
    }
} 

void zmap_insert(map *m, string *key, value *v) {
    if (m->size + 1 > m->cap * LOADFACTOR) {
        len_t oldcap = m->cap;
        entry *oldhash = m->hash;
        m->cap = m->cap<INITIALCAP?INITIALCAP:m->cap*GROWTHFACTOR;
        m->hash = ALLOCS(sizeof(entry)*m->cap);
        memset(m->hash, 0, sizeof(entry)*m->cap); /* initialize every byte to 0 effectively */
        if (!oldhash) goto end;
        for (len_t i = 0;i<oldcap;i++) {
            entry *old = oldhash + i;
            if (!old->key) continue;
            entry *n = findentry(m, old->key);
            n->key = old->key;
            n->v = old->v;
        }
        REALLOC(oldhash, oldcap, 0);
    }
    end:;
    entry *e = findentry(m, key);
    if (!e->key) m->size++;
    e->key = key;
    e->v = *v;
}

int zmap_search(map *m, string *key, value *v) {
    if (!m->size) return MAPFAILURE;
    entry *e = findentry(m, key);
    if (!e->key) return MAPFAILURE;
    *v = e->v;
    return MAPSUCCESS;
}

int zmap_delete(map *m, string *key) {
    if (!m->size) return MAPFAILURE;
    entry *e = findentry(m, key);
    if (!e->key) return MAPFAILURE;
    e->key = NULL;
    SETVAL(&e->v, 0, n, DUMMYVALUE);
    return MAPSUCCESS;
}

string *zmap_xsearch(map *m, char *str, len_t len, hash_t h) {
    if (!m->size) return NULL;
    len_t hsh = h & (m->cap-1);
    for (;;) {
        entry *e = m->hash + hsh;
        if (!e->key) return NULL;
        else {
            string *s = e->key;
            if (s->len == len && s->hash == h && !memcmp(str, s->str, len)) return s;
        }
        hsh = (hsh+1) & (m->cap-1);
    }   
}