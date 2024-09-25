#ifndef LEX_H
#define LEX_H
#include "zenVM.h"
#include <setjmp.h>

typedef enum {
    TOK_NUMBER = 256,
    TOK_NAME,
    TOK_AND,
    TOK_OR,
    TOK_EOF,
    TOK_LE,
    TOK_GE,
    TOK_EQ,
    TOK_NEQ,
    TOK_STR,
    KW_PRINT,
    KW_NULL,
    KW_TRUE,
    KW_FALSE,
    KW_VAR,
} tokentype;

#define LASTKEYWORD KW_VAR

extern const char *keywords[LASTKEYWORD-KW_PRINT+1];

typedef struct {
    int t;
    char *buf;
    int len;
    union {
        double k;
        string *buf;
    } info;
} token;

typedef struct {
    char *p;
    token tok;
    uint_t line;
    jmp_buf recovery;
    map keywords;
} zenLexer;

extern zenLexer lexer;

#define TOKBUFFSIZE sizeof(lexer.tok.info.buf)

void zlex_init(char *s);
void zlex_advance();

void zlex_error(const char *msg, const char *f, ...);

#endif