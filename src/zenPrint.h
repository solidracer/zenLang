#ifndef PRINT_H
#define PRINT_H
#include "zenLexer.h"

void zp_printvalue(value *v);
void zp_puts(const char *s);
void zp_char(char ch);

void zp_printtoken(token *t);

void zp_printbytecode(byte *code, value *k, len_t len);

#endif