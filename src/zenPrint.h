#ifndef ZPRINT_H
#define ZPRINT_H
#include "zenVMext.h"

void zp_printvalue(value *v, int nline);

void zp_dumpbytecode(byte *code, value *k);

#endif