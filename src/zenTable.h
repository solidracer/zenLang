#ifndef ZTAB_H
#define ZTAB_H
#include "zenString.h"

typedef struct {
    string *key;
    value v;
} entry;

typedef struct {
    entry *hash;
    len_t cap, size;
} table;

void ztab_init(table *t);
void ztab_free(table *t);

void ztab_insert(table *t, string *key, value *v);
int ztab_search(table *t, string *key, value *v);
int ztab_remove(table *t, string *key);

int ztab_set(table *t, string *key, value *v);

string *ztab_xsearch(table *t, char *s, len_t len, hash_t h);

/* this is very funny */
typedef struct {
    object o;
    table tab;
} otable;
#define GETTABLE(tabl) (&tabl->tab)

otable *ztab_alocnew();

#endif