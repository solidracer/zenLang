#include "zenVM.h"
#include "zenVMext.h"
#include "zenPrint.h"
#include <math.h>

#define ARITH(x) do {\
                    double right = zvm_popnum(); \
                    double left = zvm_popnum(); \
                    zvm_pushnum(left x right); \
                }while(0)

#define READSHORT() (vm.pc += 2, (unsigned short)((vm.pc[-2] << 8) | vm.pc[-1]))

static int isfalse(value *v) {
    switch(v->t) {
        case TYPE_NULL: return 1;
        case TYPE_BOOL: return !GETVAL(v, b);
        default: return 0;
    }
}

static int compare(value *v, value *v2) {
    if (v->t != v2->t) {zvm_error("TypeError", "expected types %s and %s for compare", zen_typenames[v->t], zen_typenames[v2->t]);}
    switch(v->t) {
        case TYPE_NUMBER: {
            return fabs(GETVAL(v, n) - GETVAL(v2, n)) < ZEN_EPSILON;
        }
        case TYPE_BOOL: {
            return GETVAL(v, b) == GETVAL(v2, b);
        }
        case TYPE_NULL: return 1;
        default: return GETVAL(v, o) == GETVAL(v2, o);
    }
}

zen_VM vm;

void zvm_init() {
    vm.sp = vm.stack;
    vm.allocated = 0;
    vm.objects = NULL;
    ztab_init(&vm.strings);
}

void zvm_free() {
    object *cur, *n;
    ztab_free(&vm.strings);
    for (cur = vm.objects;cur;n=cur->next,zobj_free(cur),cur=n);
}

int zvm_execute(byte *pc, value *k, int *lines) {
    vm.pc = pc;
    vm.k = k;
    vm.code = pc;
    vm.lines = lines;
    if (setjmp(vm.errbf)) return ZEN_FAILURE;
    for (;;) {
        switch((opcode)*vm.pc++) {
            case OP_HALT: return ZEN_SUCCESS;
            case OP_PRINT: {
                zp_printvalue(zvm_pop(), 1);
                continue;
            }
            case OP_PUSHK: {
                zvm_push(vm.k + *vm.pc++);
                continue;
            }
            case OP_ADD: {
                ARITH(+);
                continue;
            }
            case OP_SUB: {
                ARITH(-);
                continue;
            }
            case OP_MUL: {
                ARITH(*);
                continue;
            }
            case OP_DIV: {
                ARITH(/);
                continue;
            }
            case OP_PUSHT: {
                zvm_pushbool(1);
                continue;
            }
            case OP_JMPFPOP: {
                value *v = --vm.sp;
                unsigned short j = READSHORT();
                vm.pc += j * isfalse(v);
                continue;
            }
            case OP_PUSHF: {
                zvm_pushbool(0);
                continue;
            }
            case OP_PUSHNULL: {
                zvm_pushnull();
                continue;
            }
            case OP_JMPT: {
                unsigned short j = READSHORT();
                vm.pc += !isfalse(zvm_top()) * j;
                continue;
            }
            case OP_JMP: {
                unsigned short j = READSHORT();
                vm.pc += j;
                continue;
            }
            case OP_JMPPUSHF: {
                unsigned short j = READSHORT();
                zvm_pushbool(0);
                vm.pc += j;
                continue;
            }
            case OP_JMPLT: {
                unsigned short j = READSHORT();
                double right = zvm_popnum();
                double left = zvm_popnum();
                vm.pc += (left < right) * j;
                continue;
            }
            case OP_JMPGT: {
                unsigned short j = READSHORT();
                double right = zvm_popnum();
                double left = zvm_popnum();
                vm.pc += (left > right) * j;
                continue;
            }
            case OP_JMPLE: {
                unsigned short j = READSHORT();
                double right = zvm_popnum();
                double left = zvm_popnum();
                vm.pc += (left <= right) * j;
                continue;
            }
            case OP_JMPGE: {
                unsigned short j = READSHORT();
                double right = zvm_popnum();
                double left = zvm_popnum();
                vm.pc += (left >= right) * j;
                continue;
            }
            case OP_JMPF: {
                unsigned short j = READSHORT();
                vm.pc += isfalse(zvm_top()) * j;
                continue;
            }
            case OP_JMPBACK: {
                unsigned short j = READSHORT();
                vm.pc -= j;
                continue;
            }
            case OP_JMPEQ: {
                value *right = zvm_pop();
                value *left = zvm_pop();
                int c = compare(left, right);
                unsigned short j = READSHORT();
                vm.pc += j * c;
                continue;
            }
            case OP_JMPNEQ: {
                value *right = zvm_pop();
                value *left = zvm_pop();
                int c = compare(left, right);
                unsigned short j = READSHORT();
                vm.pc += j * !c;
                continue;
            }
            case OP_IDIV: {
                double right = zvm_popnum();
                double left = zvm_popnum();
                zvm_pushnum((int)(left / right));
                continue;
            }
            case OP_POW: {
                double right = zvm_popnum();
                double left = zvm_popnum();
                zvm_pushnum(pow(left,right));
                continue;
            }
            case OP_MOD: {
                double right = zvm_popnum();
                double left = zvm_popnum();
                zvm_pushnum(fmod(left,right));
                continue;
            }
            case OP_SETFAST: {
                vm.stack[*vm.pc++] = *(vm.sp-1);
                continue;
            }
            case OP_GETFAST: {
                zvm_push(vm.stack + *vm.pc++);
                continue;
            }
            case OP_NEG: {
                double x = zvm_popnum();
                zvm_pushnum(-x);
                continue;
            }
            case OP_POP: {
                vm.sp--;
                continue;
            }
            default: return ZEN_FAILURE;
        }
    }
}