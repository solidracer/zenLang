#include "zenCompiler.h"
#include "zenPrint.h"
#include <stdlib.h>

zenCompiler comp;

static int writebyte(byte x) {
    comp.code = zmem_allocate(comp.code, comp.len, comp.len+1);
    comp.code[comp.len] = x;
    return comp.len++; 
}

static int writebytes(byte x, byte y) {
    byte z = writebyte(x);
    writebyte(y);
    return z;
}

static int writenumber(double k) {
    comp.k = REALLOC(comp.k, comp.klen, comp.klen+1);
    value *v = &comp.k[comp.klen];
    SETVAL(v, TYPE_NUMBER, n, k);
    return comp.klen++;
}

static int writestring(string *s) {
    comp.k = REALLOC(comp.k, comp.klen, comp.klen+1);
    value *v = &comp.k[comp.klen];
    SETVAL(v, TYPE_STRING, o, s);
    return comp.klen++;
}

static void writeconstant(int k) {
    if (k>=256) {
        writebyte(OP_PUSHKLONG);
        writebytes((k >> 8) & 0xFF, k & 0xFF);
    }else writebytes(OP_PUSHK, k);
}

static int test(int c) {
    if (lexer.tok.t==c) {
        zlex_advance();
        return 1;
    }
    return 0;
}

static void strict(int c, const char *msg) {
    if (lexer.tok.t==c) {
        zlex_advance();
        return;
    }
    zlex_error("SyntaxError", msg);
}

static void check(int c, const char *msg) {
    if (lexer.tok.t!=c) zlex_error("SyntaxError", msg);
}

static binoptype getbinop(int t) {
    switch(t) {
        case '+': return OPR_ADD;
        case '-': return OPR_SUB;
        case '*': return OPR_MUL;
        case '/': return OPR_DIV;
        case '%': return OPR_MOD;
        case '^': return OPR_POW;
        case '=': return OPR_ASSIGN;
        case TOK_OR: return OPR_OR;
        case TOK_AND: return OPR_AND;
        case TOK_LE: return OPR_LE;
        case TOK_GE: return OPR_GE;
        case '<': return OPR_LT;
        case '>': return OPR_GT;
        case TOK_EQ: return OPR_EQ;
        case TOK_NEQ: return OPR_NEQ;
        default: return OPR_NONE;
    }
}

static int writejmp(byte jmp) {
    writebyte(jmp);
    return writebytes(0xFF, 0xFF);
}

static void patchjmp(int j) {
    int jump = comp.len - j - 2;
    comp.code[j] = (jump >> 8) & 0xFF;
    comp.code[j + 1] = jump & 0xff;
}

static unaryoptype getunop(int t) {
    switch(t) {
        case '+': return UOPR_PLUS;
        case '-': return UOPR_MINUS;
        case '!': return UOPR_BANG;
        default: return UOPR_NONE;
    }
}

static void addlocal(string *name) {
    for (int i = comp.loccount-1;i>=0;i--) {
        local *l = &comp.locals[i];
        if (l->depth < comp.depth) {
            break;
        }
        if (l->name==name) zlex_error("DuplicateVar", "var '%s' already exists and is duplicated", name->str);
    }
    local *l = &comp.locals[comp.loccount++];
    l->name = name;
    l->depth = comp.depth;
}

static int getlocal(string *name) {
    for (int i = comp.loccount-1;i>=0;i--) {
        local *l = &comp.locals[i];
        if (l->name == name) {
            return i;
        }
    }
    return -1;
}

static void enterscope(blockchain *b) {
    comp.depth++;
    b->prev = comp.cur;
    comp.cur = b;
}

static void leavescope() {
    blockchain *ch = comp.cur;
    comp.cur = ch->prev;
    comp.depth--;
    byte d = comp.loccount;
    while (comp.loccount > 0 && comp.locals[comp.loccount-1].depth > comp.depth) {
        comp.loccount--;
    }
    byte n = d - comp.loccount;
    if (n>0) writebytes(OP_POPN, n);
}

static void expr(exprinfo *e);

static void literal(exprinfo *e) {
    switch(lexer.tok.t) {
        case TOK_NUMBER: {
            e->t = EXP_NUMBER;
            e->info.k = lexer.tok.info.k;
            zlex_advance();
            break;
        }
        case TOK_STR: {
            e->t = EXP_DISCHARGED;
            writeconstant(writestring(lexer.tok.info.buf));
            zlex_advance();
            break;
        }
        case KW_NULL: {
            zlex_advance();
            writebyte(OP_PUSHNULL);
            e->t = EXP_DISCHARGED;
            break;
        }
        case KW_TRUE: {
            zlex_advance();
            writebyte(OP_PUSHT);
            e->t = EXP_DISCHARGED;
            break;
        }
        case KW_FALSE: {
            zlex_advance();
            writebyte(OP_PUSHF);
            e->t = EXP_DISCHARGED;
            break;
        }
        case TOK_NAME: {
            int n = getlocal(lexer.tok.info.buf);
            if (n==-1) zlex_error("UndefinedVariable", "variable '%.*s' is not defined", lexer.tok.len, lexer.tok.buf);
            zlex_advance();
            e->t = EXP_LOCAL;
            e->info.f = n;
            break;
        }
        case '(': {
            zlex_advance();
            expr(e);
            strict(')', "expected ')' to terminate '('");
            break;
        }
        default: {
            zlex_error("SyntaxError", "unexpected token '%.*s'", lexer.tok.len, lexer.tok.buf);
        }
    }
}

static void discharge(exprinfo *e) {
    switch(e->t) {
        case EXP_DISCHARGED: break;
        case EXP_NUMBER: {
            writeconstant(writenumber(e->info.k));
            e->t = EXP_DISCHARGED;
            break;
        }
        case EXP_LOCAL: {
            writebytes(OP_GETFAST, e->info.f);
            e->t = EXP_DISCHARGED;
            break;
        }
        case EXP_CMP: {
            int tj = writejmp(e->info.f);
            int fj = writejmp(OP_JMPPUSHF);
            patchjmp(tj);
            writebyte(OP_PUSHT);
            patchjmp(fj);
            e->t = EXP_DISCHARGED;
            break;
        }
    }
}

struct {
    byte left, right;
} precedence[] = {
    {4,4}, {4,4}, {5,5}, {5,5}, {5,5}, {8,7}, {8,1}, {2,2}, {1,1}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}
};
#define UNARYPREC 6

static binoptype subexpr(exprinfo *e, uint_t limit) {
    unaryoptype uop = getunop(lexer.tok.t);
    if (uop != UOPR_NONE) {
        zlex_advance();
        subexpr(e, UNARYPREC);
        discharge(e);
        switch(uop) {
            case UOPR_MINUS:
                writebyte(OP_UNM);
                break;
            case UOPR_PLUS:
                writebyte(OP_UNP);
                break;
            case UOPR_BANG:
                writebyte(OP_UNB);
                break;
            default: break;
        }
    }else literal(e);
    binoptype op = getbinop(lexer.tok.t);
    while (op != OPR_NONE && precedence[op].left > limit) {
        zlex_advance();
        exprinfo e2;
        if (op != OPR_ASSIGN) discharge(e);
        else {
            if (e->t!=EXP_LOCAL) zlex_error("SyntaxError", "expression is not assignable");
        }
        int lj;
        if (op == OPR_AND) {
            lj = writejmp(OP_JMPF);
            writebyte(OP_POP);
        }else if (op == OPR_OR) {
            lj = writejmp(OP_JMPT);
            writebyte(OP_POP);
        }
        binoptype next;
        next = subexpr(&e2, precedence[op].right);
        discharge(&e2);
        switch(op) {
            case OPR_ADD: 
                writebyte(OP_ADD);
                break;
            case OPR_SUB: 
                writebyte(OP_SUB);
                break;
            case OPR_MUL: 
                writebyte(OP_MUL);
                break;
            case OPR_DIV: 
                writebyte(OP_DIV);
                break;
            case OPR_MOD: 
                writebyte(OP_MOD);
                break;
            case OPR_POW: 
                writebyte(OP_POW);
                break;
            case OPR_ASSIGN:
                writebytes(OP_SETFAST, e->info.f);
                e->t = EXP_DISCHARGED;
                break;
            case OPR_AND:
                patchjmp(lj);
                break;
            case OPR_OR:
                patchjmp(lj);
                break;
            case OPR_EQ:
                e->t = EXP_CMP;
                e->info.f = OP_JMPEQ;
                break;
            case OPR_NEQ:
                e->t = EXP_CMP;
                e->info.f = OP_JMPNEQ;
                break;
            case OPR_LE:
                e->t = EXP_CMP;
                e->info.f = OP_JMPLE;
                break;
            case OPR_LT:
                e->t = EXP_CMP;
                e->info.f = OP_JMPLT;
                break;
            case OPR_GE:
                e->t = EXP_CMP;
                e->info.f = OP_JMPGE;
                break;
            case OPR_GT:
                e->t = EXP_CMP;
                e->info.f = OP_JMPGT;
                break;
            default: break;
        }
        op = next;
    }
    return op;
}

static void expr(exprinfo *e) {
    subexpr(e, 0);
} 

static void writestandard(FILE *out) {
    fwrite(&comp.len, sizeof(len_t), 1, out);
    fwrite(&comp.klen, sizeof(len_t), 1, out);
    fwrite(comp.code, 1, comp.len, out);
    for (len_t i = 0;i<comp.klen;i++) {
        value *k = &comp.k[i];
        fputc(k->t, out);
        switch(k->t) {
            case TYPE_NUMBER: {
                fwrite(&GETVAL(k, n), sizeof(double), 1, out);
                break;
            }
            case TYPE_STRING: {
                string *s = GETVAL(k, o);
                fwrite(&s->len, sizeof(len_t), 1, out);
                fwrite(s->str, 1, s->len, out);
                break;
            }
        }
    }
}

static void statement() {
    switch(lexer.tok.t) {
        case KW_PRINT: {
            zlex_advance();
            do {
                exprinfo e;
                expr(&e);
                discharge(&e);
                writebyte(OP_PRINT);
            } while(test(','));
            test(';');
            break;
        }
        case KW_VAR: {
            zlex_advance();
            check(TOK_NAME, "expected name for variable");
            string *name = lexer.tok.info.buf;
            zlex_advance();
            if (test('=')) {
                exprinfo e;
                expr(&e);
                discharge(&e);
            }else writebyte(OP_PUSHNULL);
            addlocal(name);
            test(';');
             break;
        }
        case '{': {
            zlex_advance();
            blockchain bl;
            enterscope(&bl);
            for (;;) {
                if (lexer.tok.t == TOK_EOF || lexer.tok.t == '}') break;
                statement();
            }
            leavescope();
            strict('}', "block must end with a '}");
            test(';');
            break;
        }
        default: {
            exprinfo e;
            expr(&e);
            discharge(&e);
            writebyte(OP_POP);
            break;
        }
    }
}

int zcomp_compile(char *out) {
    comp.loccount = comp.depth = 0;
    comp.code = NULL;
    comp.k = NULL;
    comp.len = comp.klen = 0;
    comp.cur = NULL;
    blockchain main;
    enterscope(&main);
    zlex_advance();
    int ex = setjmp(lexer.recovery);
    if (ex) return ex;
    for (;;) {
        if (lexer.tok.t==TOK_EOF) break;
        statement();
    }
    leavescope();
    writebyte(OP_HALT);
    zmap_free(&lexer.keywords);

    if (!out) return 0;
    FILE *outf = fopen(out, "wb");
    writestandard(outf);
    fclose(outf);
    return 0;
}