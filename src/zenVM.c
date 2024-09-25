#include "zenVM.h"
#include "zenPrint.h"
#include <math.h>
#include <stdio.h>

VM vm;

void zvm_init(void) {
    vm.sp = vm.stack;
    vm.allocated = 0;
    vm.objects = NULL;
    zmap_init(&vm.strings);
}

void zvm_free(void) {
    zmap_free(&vm.strings);
    for (object *o = vm.objects;o;) {
        object *n = o->next;
        zobj_free(o);
        o = n;
    }
}

#define READSHORT() (vm.pc += 2, (ushrt_t)((vm.pc[-2] << 8) | vm.pc[-1]))

static int tobool(value *v) {
    switch(v->t) {
        case TYPE_NULL: return 0;
        case TYPE_BOOL: return GETVAL(v, b);
        default: return 1;
    }
}

#define isfalse(v) (!tobool(v))

static int isequal(value *x, value *y) {
    if (x->t != y->t) return 0;
    valuetype t = x->t;
    switch(t) {
        case TYPE_NULL: return 1;
        /* for the infamous case: 0.1 + 0.2 != 0.3 */
        case TYPE_NUMBER: return fabs(GETVAL(x, n) - GETVAL(y, n)) < EPSILON;
        case TYPE_BOOL: return GETVAL(x, b) == GETVAL(y, b);
        case TYPE_STRING: return GETVAL(x, o) == GETVAL(x, o);
        default: return -1;
    }
}

/*
    null (TYPE_NULL) = ALWAYS falsy
    boolean (TYPE_BOOL) = false -> falsy, true -> truthy,
    numbers (TYPE_NUMBER) = ALWAYS truthy

    like in lisp
*/

int zvm_execute(byte *pc, value *k) {
    vm.pc = pc;
    vm.k = k;
    for (;;) {
        switch((opcode)*vm.pc++) {
            case OP_HALT: return EXECSUCCESS;
            case OP_PRINT: {
                value *v = --vm.sp;
                zp_printvalue(v);
                zp_char('\n');
                continue;
            }
            case OP_PUSHK: {
                *vm.sp++ = vm.k[*vm.pc++];
                continue;
            }
            case OP_PUSHKLONG: {
                ushrt_t k = READSHORT();
                *vm.sp++ = vm.k[k];
                continue;
            }
            case OP_PUSHNULL: {
                value *v = vm.sp++;
                SETVAL(v, TYPE_NULL, n, 0);
                continue;
            }
            case OP_PUSHT: {
                value *v = vm.sp++;
                SETVAL(v, TYPE_BOOL, b, 1);
                continue;
            }
            case OP_PUSHNULLN: {
                byte n = *vm.pc++;
                while (n--) {
                    value *v = vm.sp++;
                    SETVAL(v, TYPE_NULL, n, 0);
                }
                continue;
            }
            case OP_GETFAST: {
                byte f = *vm.pc++;
                *vm.sp++ = vm.stack[f];
                continue;
            }
            case OP_SETFAST: {
                byte f = *vm.pc++;
                vm.stack[f] = *(vm.sp-1);
                continue;
            }
            case OP_SETFASTPOP: {
                byte f = *vm.pc++;
                vm.stack[f] = *--vm.sp;
                continue;
            }
            case OP_POP: {
                vm.sp--;
                continue;
            }
            case OP_POPN: {
                byte n = *vm.pc++;
                vm.sp -= n;
                continue;
            }
            case OP_ADD: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                value *v = vm.sp++;
                SETVAL(v, TYPE_NUMBER, n, GETVAL(left, n) + GETVAL(right, n));
                continue;
            }
            case OP_SUB: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                value *v = vm.sp++;
                SETVAL(v, TYPE_NUMBER, n, GETVAL(left, n) - GETVAL(right, n));
                continue;
            }
            case OP_MUL: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                value *v = vm.sp++;
                SETVAL(v, TYPE_NUMBER, n, GETVAL(left, n) * GETVAL(right, n));
                continue;
            }
            case OP_DIV: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                value *v = vm.sp++;
                SETVAL(v, TYPE_NUMBER, n, GETVAL(left, n) / GETVAL(right, n));
                continue;
            }
            case OP_MOD: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                value *v = vm.sp++;
                SETVAL(v, TYPE_NUMBER, n, fmod(GETVAL(left, n) , GETVAL(right, n)));
                continue;
            }
            case OP_POW: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                value *v = vm.sp++;
                SETVAL(v, TYPE_NUMBER, n, pow(GETVAL(left, n) , GETVAL(right, n)));
                continue;
            }
            case OP_JMPLE: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                ushrt_t j = READSHORT();
                int c = GETVAL(left, n) <= GETVAL(right, n);
                vm.pc += c * j;
                continue;
            }
            case OP_JMPGE: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                ushrt_t j = READSHORT();
                int c = GETVAL(left, n) >= GETVAL(right, n);
                vm.pc += c * j;
                continue;
            }
            case OP_JMPLT: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                ushrt_t j = READSHORT();
                int c = GETVAL(left, n) < GETVAL(right, n);
                vm.pc += c * j;
                continue;
            }
            case OP_JMPGT: {
                value *right = --vm.sp;
                value *left = --vm.sp;
                ushrt_t j = READSHORT();
                int c = GETVAL(left, n) > GETVAL(right, n);
                vm.pc += c * j;
                continue;
            }
            case OP_UNM: {
                value *v = vm.sp - 1;
                SETVAL(v, TYPE_NUMBER, n, -GETVAL(v, n));
                continue;
            }
            case OP_UNB: {
                value *v = vm.sp - 1;
                SETVAL(v, TYPE_BOOL, b, !tobool(v));
                continue;
            }
            case OP_UNP: {
                continue;
            }
            case OP_JMP: {
                ushrt_t j = READSHORT();
                vm.pc += j;
                continue;
            }
            case OP_JMPB: {
                ushrt_t j = READSHORT();
                vm.pc -= j;
                continue;
            }
            case OP_JMPT: {
                value *v = vm.sp - 1;
                ushrt_t j = READSHORT();
                vm.pc += !isfalse(v) * j;
                continue;
            }
            case OP_JMPF: {
                value *v = vm.sp - 1;
                ushrt_t j = READSHORT();
                vm.pc += isfalse(v) * j;
                continue;
            }
            case OP_JMPPUSHF: {
                ushrt_t j = READSHORT();
                vm.pc += j;
                value *v = vm.sp++;
                SETVAL(v, TYPE_BOOL, b, 0);
                continue;
            }
            case OP_JMPPUSHT: {
                ushrt_t j = READSHORT();
                vm.pc += j;
                value *v = vm.sp++;
                SETVAL(v, TYPE_BOOL, b, 1);
                continue;
            }
            case OP_JMPPOPT: {
                ushrt_t j = READSHORT();
                value *v = --vm.sp;
                vm.pc += !isfalse(v) * j;
                continue;
            }
            case OP_JMPPOPF: {
                ushrt_t j = READSHORT();
                value *v = --vm.sp;
                vm.pc += isfalse(v) * j;
                continue;
            }
            case OP_JMPEQ: {
                ushrt_t j = READSHORT();
                value *right = --vm.sp;
                value *left = --vm.sp;
                vm.pc += isequal(left, right) * j;
                continue;
            }
            case OP_JMPNEQ: {
                ushrt_t j = READSHORT();
                value *right = --vm.sp;
                value *left = --vm.sp;
                vm.pc += !isequal(left, right) * j;
                continue;
            }
            case OP_PUSHF: {
                value *v = vm.sp++;
                SETVAL(v, TYPE_BOOL, b, 0);
                continue;
            }
            default: {
                fprintf(stderr, "illegal instruction (aborting execution)\n");
                return EXECFAILURE;
            }
        }
    }
}