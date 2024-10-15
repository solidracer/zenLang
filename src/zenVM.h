#ifndef ZVM_H
#define ZVM_H
#include "zenTable.h"
#include <setjmp.h>

typedef enum {
    OP_HALT,
    OP_PUSHK,
    OP_PRINT,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_IDIV,
    OP_POW,
    OP_MOD,
    OP_SETFAST,
    OP_GETFAST,
    OP_POP,
    OP_NEG,
    OP_NOT,
    OP_JMP,
    OP_JMPF,
    OP_JMPT,
    OP_PUSHT,
    OP_PUSHF,
    OP_PUSHNULL,
    OP_JMPPUSHF,
    OP_JMPFPOP,
    OP_JMPEQ,
    OP_JMPNEQ,
    OP_JMPBACK,
    OP_JMPLT,
    OP_JMPGT,
    OP_JMPLE,
    OP_JMPGE,
    OP_DUP,
    /* hashtable based operations */
    OP_NEWTABLE,
    OP_INSERTTABLE,
    OP_DELETETABLE,
    OP_GETTABLE,
    /* global variables */
    OP_NEWGLOBAL,
    OP_SETGLOBAL,
    OP_GETGLOBAL,
    OP_DELETEGLOBAL
} opcode;

typedef struct {
    byte *pc;
    value *k;
    value *sp;
    value stack[256];
    int *lines;
    byte *code;
    jmp_buf errbf;
    object *objects;
    size_t allocated;
    table strings;
    char *fname;
    table globals;
} zen_VM;

extern zen_VM vm;

void zvm_init();
void zvm_free();

int zvm_execute(byte *pc, value *k, int *lines);

#endif