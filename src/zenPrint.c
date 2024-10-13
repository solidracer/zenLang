#include "zenPrint.h"
#include <stdio.h>

void zp_printvalue(value *v, int nline) {
    switch(v->t) {
        case TYPE_NUMBER: {
            printf("%.14g", GETVAL(v, n));
            break;
        }
        case TYPE_BOOL: {
            printf("%s", GETVAL(v, b)?"true":"false");
            break;
        }
        case TYPE_NULL: {
            printf("null");
            break;
        }
        case TYPE_STRING: {
            string *s = GETVAL(v, o);
            printf(STRFRMT, TOFRMT(s));
            break;
        }
        default: {
            printf("<unknown type>");
            break;
        }
    }
    if (nline) putchar('\n');
}

#define GETJMP() (unsigned short)((code[i+1] << 8) | code[i+2]);
void zp_dumpbytecode(byte *code, value *k) {
    int i;
    for (i = 0;;) {
        printf("%d\t", i);
        opcode b = code[i];
        if (b==OP_HALT) {
            printf("HALT\n");
            break;
        }
        switch(b) {
            case OP_PUSHK: {
                printf("PUSHK $(");
                zp_printvalue(k + code[i+1], 0);
                printf(")\n");
                i += 2;
                continue;
            }
            case OP_PRINT: {
                printf("PRINT\n");
                i += 1;
                continue;
            }
            case OP_JMPF: {
                unsigned short j = GETJMP();
                printf("JMPF %hhu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_PUSHT: {
                printf("PUSHT\n");
                i++;
                continue;
            }
            case OP_POP: {
                printf("POP\n");
                i++;
                continue;
            }
            case OP_PUSHF: {
                printf("PUSHF\n");
                i++;
                continue;
            }
            case OP_ADD: {
                printf("ADD\n");
                i++;
                continue;
            }
            case OP_SUB: {
                printf("SUB\n");
                i++;
                continue;
            }
            case OP_MUL: {
                printf("MUL\n");
                i++;
                continue;
            }
            case OP_DIV: {
                printf("DIV\n");
                i++;
                continue;
            }
            case OP_IDIV: {
                printf("IDIV\n");
                i++;
                continue;
            }
            case OP_MOD: {
                printf("MOD\n");
                i++;
                continue;
            }
            case OP_POW: {
                printf("POW\n");
                i++;
                continue;
            }
            case OP_SETFAST: {
                byte arg = code[i+1];
                printf("SETFAST %hhu\n", arg);
                i += 2;
                continue;
            }
            case OP_GETFAST: {
                byte arg = code[i+1];
                printf("GETFAST %hhu\n", arg);
                i += 2;
                continue;
            }
            case OP_NEG: {
                printf("NEG\n");
                i++;
                continue;
            }
            case OP_JMP: {
                unsigned short j = GETJMP();
                printf("JMP %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPT: {
                unsigned short j = GETJMP();
                printf("JMPT %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_PUSHNULL: {
                printf("PUSHNULL\n");
                i++;
                continue;
            }
            case OP_JMPPUSHF: {
                unsigned short j = GETJMP();
                printf("JMPPUSHF %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPFPOP: {
                unsigned short j = GETJMP();
                printf("JMPFPOP %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPEQ: {
                unsigned short j = GETJMP();
                printf("JMPEQ %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPNEQ: {
                unsigned short j = GETJMP();
                printf("JMPNEQ %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPBACK: {
                unsigned short j = GETJMP();
                printf("JMPBACK %hu (%d)\n", j, i + 3 - j);
                i += 3;
                continue;
            }
            case OP_JMPLT: {
                unsigned short j = GETJMP();
                printf("JMPLT %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPGT: {
                unsigned short j = GETJMP();
                printf("JMPGT %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPLE: {
                unsigned short j = GETJMP();
                printf("JMPLE %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_JMPGE: {
                unsigned short j = GETJMP();
                printf("JMPGE %hu (%d)\n", j, i + j + 3);
                i += 3;
                continue;
            }
            case OP_HALT: break;
        }
    }
}