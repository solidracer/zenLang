#ifndef COMP_H
#define COMP_H
#include "zenLexer.h"
#include <stdio.h> /* for writing */

typedef struct bch {
    struct bch *prev;
} blockchain;

typedef struct {
    uint_t depth;
    string *name;
} local;

typedef enum {
    EXP_DISCHARGED, /* doesnt need to discharge, or already did */
    EXP_NUMBER, /* number constant */
    EXP_LOCAL, /* fast storage in stack */
    EXP_CMP, /* compare jump */
} exprtype;

typedef struct {
    exprtype t;
    union {
        double k; /* double info (for number constants) */
        byte f; /* byte info (jump type or fast offset) */
    } info;
} exprinfo;

typedef enum {
    OPR_ADD,
    OPR_SUB,
    OPR_MUL,
    OPR_DIV,
    OPR_MOD,
    OPR_POW,
    OPR_ASSIGN,
    OPR_AND,
    OPR_OR,
    OPR_EQ,
    OPR_NEQ,
    OPR_LE,
    OPR_LT,
    OPR_GE,
    OPR_GT,
    OPR_NONE,
} binoptype;

typedef enum {
    UOPR_MINUS,
    UOPR_PLUS,
    UOPR_BANG,
    UOPR_NONE,
} unaryoptype;

typedef struct {
    byte *code;
    value *k;
    len_t len, klen;
    uint_t depth;
    byte loccount;
    local locals[256];
    blockchain *cur;
} zenCompiler;

extern zenCompiler comp;

int zcomp_compile(char *out);

#endif