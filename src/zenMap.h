#ifndef MAP_H
#define MAP_H
#include "zenString.h"

typedef struct {
    string *key;
    value v;
} entry;

typedef struct {
    entry *hash;
    len_t cap, size;
} map;

#define INITIALCAP 8
#define GROWTHFACTOR 2
#define LOADFACTOR 0.75

#define MAPSUCCESS 0
#define MAPFAILURE 1

/* non zero numerical value for deleted entries */
#define DUMMYVALUE 1

void zmap_init(map *m);
void zmap_free(map *m);

void zmap_insert(map *m, string *key, value *v);
int zmap_search(map *m, string *key, value *v);
int zmap_delete(map *m, string *key);

string *zmap_xsearch(map *m, char *str, len_t len, hash_t h);

#endif