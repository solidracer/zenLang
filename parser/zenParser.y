%{
    /* this is my first time using flex and bison */
    #include "zenBytecode.h"
    #include <stdio.h>    
    int yylex(void);
    void yyerror(const char *s, ...);

    typedef struct {
        string *name;
        uint_t depth;
        byte isconst;
    } local;
    static local locals[256];
    static int loccount;
    static uint_t depth;

    static int _addlocal(string *s, byte isconst) {
        if (loccount >= 256) {
            yyerror("local stack overflow (257+ locals)");
            return 1;
        }
        local *l = &locals[loccount++];
        l->depth = depth;
        l->name = s;
        l->isconst = isconst;
        return 0;
    }
    #define addlocal(name, isconst) if (_addlocal(name, isconst)) YYERROR
    
    static int findlocal(string *s) {
        int i;
        for (i = loccount-1;i>=0;i--) {
            local *l = &locals[i];
            if (l->name == s) return i; 
        }
        return -1;
    }

    static void enterdepth() {
        depth++;
    }
    
    static void leavedepth() {
        depth--;
        while (loccount > 0 && locals[loccount-1].depth > depth) {
            loccount--;
            zc_codebyte(OP_POP);
        }
    }

    static void removevars(uint_t d) {
        int loc = loccount;
        while (loc > 0 && locals[loc-1].depth > d-1) {
            loc--;
            zc_codebyte(OP_POP);
        }
    }

    typedef struct bl {
        byte isloop;
        int bj;
        int start;
    } blockinfo;
    static blockinfo blocks[30];
    static blockinfo *block = blocks;

    #define getblock() (block-1)
    /*static blockinfo *getblock() {
        return block - 1;
    }*/

    static blockinfo *getloop() {
        blockinfo *cur = getblock();
        while ((cur-blocks)>=0 && !cur->isloop) cur -= 1;
        return cur->isloop?cur:NULL;
    }

    #define getdepth(bl) ((bl-blocks)+1)
    /*static int getdepth(blockinfo *bl) {
        return (bl-blocks)+1;
    }*/

    static int _enterblock(byte isloop) {
        if ((block-blocks) >= 30) {
            yyerror("too much depth (block stack overflow)");
            return 1;
        }
        blockinfo *bl = block++;
        bl->isloop = isloop;
        bl->bj = -1;
        enterdepth();
        return 0;
    }

    #define enterblock(isloop) if (_enterblock(isloop)) YYERROR
    
    static void leaveblock() {
        blockinfo *b = --block;
        leavedepth();
        if (b->bj != -1) {
            zc_patchjmp(b->bj);
        }
    }
%}

%code requires {
    #include "zenVM.h"
}

%union {
    double number;
    string *buffer;
    int i;
    int i2[3];
}

%token <number> NUMBER
%token <buffer> STRING NAME
%token IDIV VAR PRINT TNULL TTRUE TFALSE GE LE EQ NEQ BREAK CONTINUE CONST
%token <i> AND OR
%token <i2> WHILE IF ELSE

%precedence '='
%left OR
%left AND
%left '<' '>' GE LE EQ NEQ
%left '+' '-'
%left '*' '/' '%' IDIV
%precedence UNARY
%right '^'

%start program

%%

program: 
    { enterblock(0); }
    stats
    { leaveblock(); zc_codebyte(OP_HALT); }
    ;

stats: %empty | stats stat ;

stat:
    ';'
    | expr ';' {zc_codebyte(OP_POP);}
    | PRINT expr ';' {zc_codebyte(OP_PRINT);}
    | var_stat
    | while
    | break ';'
    | continue ';'
    | block
    | ifstat
    ;

var_stat:
    VAR NAME var_opt ';' {addlocal($2, 0);}
    | CONST VAR NAME var_opt ';' {addlocal($3, 1);}
    ;

var_opt: %empty {zc_codebyte(OP_PUSHNULL);}
    | '=' expr

/* this is meant to be a block statement lol */
block:
    '{' { enterblock(0); }
    stats
    '}' { leaveblock(); }
    ;

ifstat:
    IF cond {
        $1[0] = zc_codejmp(OP_JMPFPOP);
    }
    '{' { enterblock(0); }
    stats 
    '}' { leaveblock(); $1[1] = zc_codejmp(OP_JMP); zc_patchjmp($1[0]); }
    ifelse { zc_patchjmp($1[1]); }
    ;

ifelse: %empty |
    ELSE
    '{' {
        enterblock(0);
    }
    stats
    '}' {
        leaveblock();
    }
    | ELSE ifstat
    ;

while:
    WHILE {
        $1[0] = mchunk.len;
    } cond {
        $1[1] = zc_codejmp(OP_JMPFPOP);
    } 
    '{' { enterblock(1); getblock()->start = $1[0]; }
    stats
    '}' { $1[2] = getblock()->bj; leaveblock(); }
    {
        zc_patchloop($1[0]);
        zc_patchjmp($1[1]);
        if ($1[2]!=-1) zc_patchjmp($1[2]);
    }
    ;

break:
    BREAK {
        blockinfo *loop = getloop();
        if (!loop) {
            yyerror("cannot break out of a non loop block");
            YYERROR;
        }
        removevars(getdepth(loop));
        if (loop->bj==-1)
            loop->bj = zc_codejmp(OP_JMP);
        else {
            zc_patchjmp(loop->bj);
            loop->bj = zc_codejmp(OP_JMP);
        }
    } 
    ;

continue:
    CONTINUE {
        blockinfo *loop = getloop();
        if (!loop) {
            yyerror("cannot continue a non loop block");
            YYERROR;
        }
        removevars(getdepth(loop));
        zc_patchloop(loop->start);
    }
    ;

cond:
    '(' expr ')'
    ;

expr:
    expr '+' expr {zc_codebyte(OP_ADD);}
    | expr '-' expr {zc_codebyte(OP_SUB);}
    | expr '*' expr {zc_codebyte(OP_MUL);}
    | expr '/' expr {zc_codebyte(OP_DIV);}
    | expr '%' expr {zc_codebyte(OP_MOD);}
    | expr IDIV expr {zc_codebyte(OP_IDIV);}
    | expr '^' expr {zc_codebyte(OP_POW);}
    | expr '<' expr {
        int cond = zc_codejmp(OP_JMPLT);
        int jmp = zc_codejmp(OP_JMPPUSHF);
        zc_patchjmp(cond);
        zc_codebyte(OP_PUSHT);
        zc_patchjmp(jmp);
    }
    | expr '>' expr {
        int cond = zc_codejmp(OP_JMPGT);
        int jmp = zc_codejmp(OP_JMPPUSHF);
        zc_patchjmp(cond);
        zc_codebyte(OP_PUSHT);
        zc_patchjmp(jmp);
    }
    | expr LE expr {
        int cond = zc_codejmp(OP_JMPLE);
        int jmp = zc_codejmp(OP_JMPPUSHF);
        zc_patchjmp(cond);
        zc_codebyte(OP_PUSHT);
        zc_patchjmp(jmp);
    }
    | expr GE expr {
        int cond = zc_codejmp(OP_JMPGE);
        int jmp = zc_codejmp(OP_JMPPUSHF);
        zc_patchjmp(cond);
        zc_codebyte(OP_PUSHT);
        zc_patchjmp(jmp);
    }
    | expr EQ expr {
        int cond = zc_codejmp(OP_JMPEQ);
        int jmp = zc_codejmp(OP_JMPPUSHF);
        zc_patchjmp(cond);
        zc_codebyte(OP_PUSHT);
        zc_patchjmp(jmp);
    }
    | expr NEQ expr {
        int cond = zc_codejmp(OP_JMPNEQ);
        int jmp = zc_codejmp(OP_JMPPUSHF);
        zc_patchjmp(cond);
        zc_codebyte(OP_PUSHT);
        zc_patchjmp(jmp);
    }
    | expr AND {
        $2 = zc_codejmp(OP_JMPF);
        zc_codebyte(OP_POP);
    } expr {
        zc_patchjmp($2);
    }
    | expr OR {
        $2 = zc_codejmp(OP_JMPT);
        zc_codebyte(OP_POP);
    } expr {
        zc_patchjmp($2);
    }
    | NAME '=' expr {
        int i = findlocal($1);
        if (i == -1) {
            yyerror("variable '" STRFRMT "' does not exist", TOFRMT($1));
            YYERROR;
        }
        if (locals[i].isconst) {
            yyerror("variable '" STRFRMT "' is constant", TOFRMT($1));
            YYERROR;
        }
        zc_codelonginst(OP_SETFAST, i);
    }
    | '-' expr %prec UNARY {zc_codebyte(OP_NEG);}
    | '(' expr ')' {}
    | NUMBER {zc_codenum($1);}
    | STRING {zc_codestr($1);}
    | TFALSE {zc_codebyte(OP_PUSHF);}
    | TTRUE {zc_codebyte(OP_PUSHT);}
    | TNULL {zc_codebyte(OP_PUSHNULL);}
    | NAME {
        int i = findlocal($1);
        if (i == -1) {
            yyerror("variable '" STRFRMT "' does not exist", TOFRMT($1));
            YYERROR;
        }
        zc_codelonginst(OP_GETFAST, i);
    }
    ;

%%

/* ZENLANG PARSER END, WRITTEN BY SOLIDRACER */
/* COMPILER FOR ZVM (zenVM) */