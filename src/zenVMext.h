#ifndef ZVMEXT_H
#define ZVMEXT_H
#include "zenVM.h"

double zvm_popnum();
int zvm_popbool();

value *zvm_top();
value *zvm_peek(int i);

void zvm_setnum(double x);
void zvm_setbool(int b);
void zvm_setnull();

value *zvm_pop();
void zvm_push(value *v);

void zvm_pushnum(double n);
void zvm_pushbool(int b);
void zvm_pushnull();

void zvm_error(const char *msg, const char *f, ...);

#endif