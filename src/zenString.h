#ifndef STR_H
#define STR_H
#include "zenObject.h"

typedef struct {
    object o;
    len_t len;
    hash_t hash;
    char str[];
} string;

string *zstr_new(char *buf, len_t len);
hash_t zstr_hash(char *s, len_t len);

#define K2STR(k) zstr_new(k, sizeof(k)-1)

#endif