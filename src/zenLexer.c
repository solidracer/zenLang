#include "zenLexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

const char *keywords[LASTKEYWORD-KW_PRINT+1] = {
    "(keyword)print",
    "(keyword)null",
    "(keyword)true",
    "(keyword)false",
    "(keyword)var",
};

zenLexer lexer;

void zlex_init(char *s) {
    lexer.p = s;
    lexer.line = 1;
    zmap_init(&lexer.keywords);
    zmap_insert(&lexer.keywords, K2STR("print"), &(value){TYPE_NUMBER, {.n = KW_PRINT}});
    zmap_insert(&lexer.keywords, K2STR("var"), &(value){TYPE_NUMBER, {.n = KW_VAR}});
    zmap_insert(&lexer.keywords, K2STR("null"), &(value){TYPE_NUMBER, {.n = KW_NULL}});
    zmap_insert(&lexer.keywords, K2STR("true"), &(value){TYPE_NUMBER, {.n = KW_TRUE}});
    zmap_insert(&lexer.keywords, K2STR("false"), &(value){TYPE_NUMBER, {.n = KW_FALSE}});
}

static int getkw(string *s) {
    value v;
    return zmap_search(&lexer.keywords, s, &v)==MAPFAILURE?-1:GETVAL(&v, n);
}

void zlex_error(const char *msg, const char *f, ...) {
    va_list args;
    va_start(args, f);
    fprintf(stderr, "\033[31m%s:\033[0m ", msg);
    vfprintf(stderr, f, args);
    fprintf(stderr, " at line %u\n", lexer.line);
    va_end(args);
    longjmp(lexer.recovery, 1);
}

static int isnline() {
    return *lexer.p == '\r' || *lexer.p == '\n';
}

static void eatnewline() { /* I am sure "r" mode already changes windows newline sequence to a '\n' but... being sure is good*/
    char c = *lexer.p++;
    if (isnline() && *lexer.p != c) lexer.p++;
    lexer.line++;
}

void zlex_advance() {
    for (;;) {
        switch(*lexer.p) {
            case '\n':
            case '\r': {
                eatnewline();
                continue;
            }
            case '&': {
                lexer.tok.buf = lexer.p;
                lexer.p++;
                if (*lexer.p == '&') {
                    lexer.p++;
                    lexer.tok.t = TOK_AND;
                    lexer.tok.len = 2;
                    return;
                }
                lexer.tok.t = '&';
                lexer.tok.len = 1;
                return;
            }
            case '|': {
                lexer.tok.buf = lexer.p;
                lexer.p++;
                if (*lexer.p == '|') {
                    lexer.p++;
                    lexer.tok.t = TOK_OR;
                    lexer.tok.len = 2;
                    return;
                }
                lexer.tok.t = '|';
                lexer.tok.len = 1;
                return;
            }
            case '<': {
                lexer.tok.buf = lexer.p;
                lexer.p++;
                if (*lexer.p == '=') {
                    lexer.tok.t = TOK_LE;
                    lexer.tok.len = 2;
                    lexer.p++;
                    return;
                }
                lexer.tok.t = '<';
                lexer.tok.len = 1;
                return;
            }
            case '>': {
                lexer.tok.buf = lexer.p;
                lexer.p++;
                if (*lexer.p == '=') {
                    lexer.tok.t = TOK_GE;
                    lexer.tok.len = 2;
                    lexer.p++;
                    return;
                }
                lexer.tok.t = '>';
                lexer.tok.len = 1;
                return;
            }
            case '!': {
                lexer.tok.buf = lexer.p;
                lexer.p++;
                if (*lexer.p == '=') {
                    lexer.tok.t = TOK_NEQ;
                    lexer.tok.len = 2;
                    lexer.p++;
                    return;
                }
                lexer.tok.t = '!';
                lexer.tok.len = 1;
                return;
            }
            case '=': {
                lexer.tok.buf = lexer.p;
                lexer.p++;
                if (*lexer.p == '=') {
                    lexer.tok.t = TOK_EQ;
                    lexer.tok.len = 2;
                    lexer.p++;
                    return;
                }
                lexer.tok.t = '=';
                lexer.tok.len = 1;
                return;
            }
            case '\'':
            case '\"': {
                lexer.tok.buf = lexer.p;
                char t = *lexer.p++;
                while (*lexer.p != t && *lexer.p != '\n' && *lexer.p != '\0') {
                    lexer.p++;
                }
                if (*lexer.p != t) zlex_error("SyntaxError", "unterminated string");
                lexer.p++;
                len_t len = lexer.p - lexer.tok.buf;
                lexer.tok.len = len;
                lexer.tok.t = TOK_STR;
                lexer.tok.info.buf = zstr_new(lexer.tok.buf + 1, len - 2);
                return;
            }
            case '\0': {
                lexer.tok.t = TOK_EOF;
                return;
            }
            case '#': {
                while (*lexer.p != '\n' && *lexer.p != '\0') lexer.p++;
                continue;
            }
            default: {
                lexer.tok.buf = lexer.p;
                char c = *lexer.p++;
                if (isalpha(c) || c == '_') {
                    while(isalnum(*lexer.p) || *lexer.p == '_') lexer.p++;
                    uint_t len = lexer.p - lexer.tok.buf;
                    lexer.tok.len = len;
                    lexer.tok.t = TOK_NAME;
                    string *s = zstr_new(lexer.tok.buf, len);
                    int kw = getkw(s);
                    if (kw!=-1) {
                        lexer.tok.t = kw;
                    }else lexer.tok.info.buf = s;
                    return;
                }else if (isdigit(c) || c == '.') {
                    while(isalnum(*lexer.p) || *lexer.p == '.') lexer.p++;
                    char *end;
                    double k = strtod(lexer.tok.buf, &end);
                    if (end != lexer.p)
                        zlex_error("MalformedNumber", "malformed number '%.*s'", (int)(lexer.p-lexer.tok.buf), lexer.tok.buf);
                    lexer.tok.t = TOK_NUMBER;
                    lexer.tok.info.k = k;
                    lexer.tok.len = lexer.p - lexer.tok.buf;
                    return;
                }else if (ispunct(c)) {
                    lexer.tok.t = c;
                    lexer.tok.len = 1;
                    return;
                }
                continue;
            }
        }
    }
}