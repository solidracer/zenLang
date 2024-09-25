#include "zenPrint.h"
#include <stdio.h>

void zp_printvalue(value *v) {
    switch(v->t) {
        case TYPE_NUMBER: {
            printf("%.14g", GETVAL(v, n));
            break;
        }
        case TYPE_NULL: {
            printf("null");
            break;
        }
        case TYPE_BOOL: {
            printf("%s", GETVAL(v, b)?"true":"false");
            break;
        }
        case TYPE_STRING: {
            string *s = GETVAL(v, o);
            printf("%.*s", s->len, s->str);
            break;
        }
    }
}

void zp_puts(const char *s) {
    printf("zen: %s\n", s);
}

void zp_char(char ch) {
    putchar(ch);
}

void zp_printtoken(token *t) {
    switch(t->t) {
        case TOK_EOF: {
            printf("eof");
            break;
        }
        case TOK_NUMBER: {
            printf("%g", t->info.k);
            break;
        }
        case TOK_NAME: {
            printf("%.*s", t->info.buf->len, t->info.buf->str);
            break;
        }
        default: {
            if (t->t>=KW_PRINT) printf("%s", keywords[t->t-KW_PRINT]);
            else printf("%c", t->t);
            break;
        }
    }
}

void zp_printbytecode(byte *code, value *k, len_t len) {
    len_t o = 0;
    for (len_t i = 0;i<len;) {
        printf("%u\t(%u.)\t\t", i, ++o);
        opcode op = code[i];
        switch(op) {
            case OP_HALT: {
                printf("HALT\n");
                i++;
                break;
            }
            case OP_PRINT: {
                printf("PRINT\n");
                i++;
                break;
            }
            case OP_PUSHF: {
                printf("PUSHF\n");
                i++;
                break;
            }
            case OP_PUSHT: {
                printf("PUSHT\n");
                i++;
                break;
            }
            case OP_PUSHNULL: {
                printf("PUSHNULL\n");
                i++;
                break;
            }
            case OP_PUSHK: {
                byte a = code[i+1];
                printf("PUSHK $(");
                zp_printvalue(k + a);
                printf(")\n");
                i += 2;
                break;
            }
            case OP_PUSHKLONG: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("PUSHKLONG $(");
                zp_printvalue(k + a);
                printf(")\n");
                i += 3;
                break;
            }
            case OP_PUSHNULLN: {
                byte a = code[i+1];
                printf("PUSHNULLN %hhu\n", a);
                i += 2;
                break;
            }
            case OP_ADD: {
                printf("ADD\n");
                i++;
                break;
            }
            case OP_SUB: {
                printf("SUB\n");
                i++;
                break;
            }
            case OP_MUL: {
                printf("MUL\n");
                i++;
                break;
            }
            case OP_DIV: {
                printf("DIV\n");
                i++;
                break;
            }
            case OP_MOD: {
                printf("MOD\n");
                i++;
                break;
            }
            case OP_POW: {
                printf("POW\n");
                i++;
                break;
            }
            case OP_UNM: {
                printf("UNM\n");
                i++;
                break;
            }
            case OP_UNP: {
                printf("UNP\n");
                i++;
                break;
            }
            case OP_UNB: {
                printf("UNB\n");
                i++;
                break;
            }
            case OP_POP: {
                printf("POP\n");
                i++;
                break;
            }
            case OP_POPN: {
                byte a = code[i+1];
                printf("POPN %hhu\n", a);
                i += 2;
                break;
            }
            case OP_SETFASTPOP: {
                byte a = code[i+1];
                printf("SETFASTPOP %hhu\n", a);
                i += 2;
                break;
            }
            case OP_SETFAST: {
                byte a = code[i+1];
                printf("SETFAST %hhu\n", a);
                i += 2;
                break;
            }
            case OP_GETFAST: {
                byte a = code[i+1];
                printf("GETFAST %hhu\n", a);
                i += 2;
                break;
            }
            case OP_JMP: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMP %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPB: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPB %hu (%llu)\n", a, (size_t)i + 3 - a);
                i += 3;
                break;
            }
            case OP_JMPT: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPT %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPF: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPF %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPLE: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPLE %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPGE: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPGE %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPGT: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPGT %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPLT: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPLT %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPPOPF: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPPOPF %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPPOPT: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPPOPT %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPPUSHT: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPPUSHT %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPEQ: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPEQ %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPNEQ: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPNEQ %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
            case OP_JMPPUSHF: {
                ushrt_t a = (ushrt_t)((code[i+1] << 8) | code[i+2]);
                printf("JMPPUSHF %hu (%llu)\n", a, (size_t)a + i + 3);
                i += 3;
                break;
            }
        }
    }
}