#include "zenVMext.h"
#include "zenPrint.h"
#include <stdio.h>
#include <stdarg.h>

double zvm_popnum() {
    value *v = --vm.sp;
    if (v->t != TYPE_NUMBER) zvm_error("TypeError", "expected number got %s", zen_typenames[v->t]);
    return GETVAL(v, n);
}

int zvm_popbool() {
    value *v = --vm.sp;
    if (v->t != TYPE_BOOL) zvm_error("TypeError", "expected bool got %s", zen_typenames[v->t]);
    return GETVAL(v, b);
}

value *zvm_top() {
    return zvm_peek(-1);
}

value *zvm_peek(int i) {
    return vm.sp + i;
}

value *zvm_pop() {
    return --vm.sp;
}

void zvm_push(value *v) {
    *vm.sp++ = *v;
}

void zvm_pushnum(double n) {
    value *v = vm.sp++;
    SETVAL(v, TYPE_NUMBER, n, n);
}

void zvm_pushbool(int b) {
    value *v = vm.sp++;
    SETVAL(v, TYPE_BOOL, b, b);
}

void zvm_pushnull() {
    value *v = vm.sp++;
    SETVAL(v, TYPE_NULL, n, 0);
}

void zvm_error(const char *msg, const char *f, ...) {
    va_list args;
    int line = vm.lines[vm.pc-vm.code];
    fprintf(stderr, "%s:%d: %s: ", vm.fname, line, msg);
    va_start(args, f);
    vfprintf(stderr, f, args);
    fprintf(stderr, " at line %d\n", line);
    va_end(args);
    longjmp(vm.errbf, 1);
}

void zvm_setnum(double x) {
    value *v = zvm_top();
    SETVAL(v, TYPE_NUMBER, n, x);
}

void zvm_setbool(int b) {
    value *v = zvm_top();
    SETVAL(v, TYPE_BOOL, b, b);
}

void zvm_setnull() {
    value *v = zvm_top();
    SETVAL(v, TYPE_NULL, n, 0);
}