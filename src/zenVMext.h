#ifndef ZVMEXT_H
#define ZVMEXT_H
#include "zenVM.h"

double zvm_popnum();
int zvm_popbool();
string *zvm_popstr();

value *zvm_top();
value *zvm_peek(int i);

void zvm_setnum(double x);
void zvm_setbool(int b);
void zvm_setnull();
void zvm_setstr(string *s);

value *zvm_pop();
void zvm_push(value *v);

void zvm_pushnum(double n);
void zvm_pushbool(int b);
void zvm_pushnull();
void zvm_pushstr(string *s);

void zvm_error(const char *msg, const char *f, ...);

#endif