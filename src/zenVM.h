#ifndef VM_H
#define VM_H
#include "zenMap.h"

typedef enum {
    OP_HALT,
    OP_PRINT,
    OP_PUSHK,
    OP_PUSHNULL,
    OP_PUSHT,
    OP_PUSHF,
    /* arithmetic */
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,
    OP_UNM,
    OP_UNP,
    OP_UNB,
    /* jumps */
    OP_JMP,
    OP_JMPB,
    OP_JMPT,
    OP_JMPF,
    OP_JMPEQ,
    OP_JMPNEQ,
    OP_JMPLE,
    OP_JMPGE,
    OP_JMPLT,
    OP_JMPGT,
    OP_JMPPUSHF,
    OP_JMPPUSHT,
    OP_JMPPOPT,
    OP_JMPPOPF,
    /* locals (fasts) */
    OP_SETFAST,
    OP_GETFAST,
    OP_SETFASTPOP,
    /* stack */
    OP_POP,
    OP_POPN,
    /* others */
    OP_PUSHKLONG,
    OP_PUSHNULLN,
} opcode;

typedef struct {
    value stack[256];
    value *sp;
    value *k;
    byte *pc;
    size_t allocated;
    object *objects;
    map strings;
} VM;

extern VM vm;

void zvm_init(void);
void zvm_free(void);

#define EXECSUCCESS 0
#define EXECFAILURE 1

int zvm_execute(byte *pc, value *k);

#endif