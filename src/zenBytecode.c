#include "zenBytecode.h"

extern int yylineno;

chunk mchunk;

void zc_init() {
    mchunk.len = mchunk.klen = 0;
    mchunk.code = NULL;
    mchunk.lines = NULL;
    mchunk.k = NULL;
}

void zc_codebyte(byte b) {
    mchunk.code = REALLOC(mchunk.code, mchunk.len, mchunk.len+1);
    mchunk.lines = REALLOC(mchunk.lines, mchunk.len, mchunk.len+1);
    mchunk.code[mchunk.len] = b;
    mchunk.lines[mchunk.len] = yylineno;
    mchunk.len++;
}

void zc_codelonginst(byte i, byte o) {
    zc_codebyte(i);
    zc_codebyte(o);
}

void zc_codeint(int n) {
    value *v;
    mchunk.k = REALLOC(mchunk.k, mchunk.klen, mchunk.klen+1);
    v = &mchunk.k[mchunk.klen];
    SETVAL(v, TYPE_NUMBER, n, n);
    zc_codelonginst(OP_PUSHK, mchunk.klen++);
}

void zc_codenum(double d) {
    value *v;
    mchunk.k = REALLOC(mchunk.k, mchunk.klen, mchunk.klen+1);
    v = &mchunk.k[mchunk.klen];
    SETVAL(v, TYPE_NUMBER, n, d);
    zc_codelonginst(OP_PUSHK, mchunk.klen++);
}

void zc_codestr(string *s) {
    value *v;
    mchunk.k = REALLOC(mchunk.k, mchunk.klen, mchunk.klen+1);
    v = &mchunk.k[mchunk.klen];
    SETVAL(v, TYPE_STRING, o, s);
    zc_codelonginst(OP_PUSHK, mchunk.klen++);
}

int zc_codejmp(opcode op) {
    zc_codebyte(op);
    zc_codelonginst(0xff, 0xff);
    return mchunk.len - 2;
}

void zc_patchjmp(int jmp) {
    int j = mchunk.len - jmp - 2;
    mchunk.code[jmp] = (j >> 8) & 0xff;
    mchunk.code[jmp+1] = j & 0xff; 
}

void zc_patchloop(int where) {
    zc_codebyte(OP_JMPBACK);
    int j = mchunk.len - where + 2;
    zc_codebyte((j >> 8) & 0xff);
    zc_codebyte(j & 0xff);
}

void zc_cleanup() {
    REALLOC(mchunk.code, mchunk.len, 0);
    REALLOC(mchunk.k, mchunk.klen, 0);
    REALLOC(mchunk.lines, mchunk.len, 0);
}