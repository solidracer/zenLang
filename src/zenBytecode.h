#ifndef ZBYT_H
#define ZBYT_H
#include "zenVM.h"

typedef struct bytecodearray {
    byte *code;
    uint_t len;
    value *k;
    uint_t klen;
    int *lines;
} chunk;

extern chunk mchunk;

void zc_init();
void zc_codebyte(byte b);
void zc_codelonginst(byte i, byte o);
void zc_codeint(int n);
void zc_codenum(double d);
void zc_codestr(string *s);
int zc_codejmp(opcode op);
void zc_patchjmp(int jmp);
void zc_patchloop(int where);
void zc_cleanup();

#endif
