#ifndef ZSTR_H
#define ZSTR_H
#include "zenObject.h"

typedef struct {
    object o;
    len_t len;
    hash_t hash;
    char *str;
} string;

string *zstr_new(char *b, len_t len);
string *zstr_take(char *b, len_t len);

#define K2STR(str) zstr_new(str, sizeof(str)-1)
#define TOFRMT(s) (int)(s)->len, (s)->str
#define STRFRMT "%.*s"

#endif