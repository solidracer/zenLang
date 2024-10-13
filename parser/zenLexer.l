%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <ctype.h>
    #include <signal.h>
    #include <stdarg.h>
    #include "zenParser.h"
    #include "zenPrint.h"
    #include "zenBytecode.h"

    static char *buffer;
    static int size, cap;

    static void addtobuf(char c) {
        if (size >= cap) {
            int old = cap;
            cap = cap<16?16:cap*2;
            buffer = REALLOC(buffer, old, cap);
        }
        buffer[size++] = c;
    }

    static void freebuf() {
        if (!buffer) return;
        REALLOC(buffer, cap, 0);
        cap = size = 0;
        buffer = NULL;
    }
%}

%option noyywrap
%option yylineno
%x STRLEX

%%
\" {BEGIN(STRLEX);}
<STRLEX>{
\\n {addtobuf('\n');}
\\t {addtobuf('\t');}
\\f {addtobuf('\f');}
\\v {addtobuf('\v');}
\\b {addtobuf('\b');}
\\r {addtobuf('\r');}
\\a {addtobuf('\a');}
\\\\ {addtobuf('\\');}
\\\" {addtobuf('\"');}
\\\' {addtobuf('\'');}
\\x[0-9a-fA-F]{2} {
    char *end; 
    byte hex = strtol(yytext + 2, &end, 16);
    addtobuf(hex);
}
\" {BEGIN(INITIAL); yylval.buffer = zstr_new(buffer, size); freebuf(); return STRING;}
. {addtobuf(yytext[0]);}
}
"//" {return IDIV;}
#.* {}
([0-9]+(\.[0-9]*)?|\.[0-9]+)([eE][+-]?[0-9]+)? {char *e; yylval.number = strtod(yytext, &e); return NUMBER;}
null {return TNULL;}
true {return TTRUE;}
false {return TFALSE;}
print {return PRINT;}
var {return VAR;}
while {return WHILE;}
break {return BREAK;}
const {return CONST;}
continue {return CONTINUE;}
if {return IF;}
else {return ELSE;}
"&&" {return AND;}
"||" {return OR;}
"<=" {return LE;}
">=" {return GE;}
"==" {return EQ;}
"!=" {return NEQ;}
[a-zA-Z_][0-9a-zA-Z_]* {yylval.buffer = zstr_new(yytext, yyleng); return NAME; }
[ \t\n] {}
. {return yytext[0];}
%%

void yyerror(const char *s, ...) {
    va_list args;
    va_start(args, s);
    fprintf(stderr, "%s:%d: error: ", vm.fname, yylineno);
    vfprintf(stderr, s, args);
    fprintf(stderr, " at line %d\n", yylineno);
    va_end(args);
}

void signalhandler(int s) {
    if (s == SIGSEGV) {
        fprintf(stderr, "seg fault.\n");
        exit(ZEN_FAILURE);
    }else if (s == SIGINT) {
        fprintf(stderr, "RECEIVED SIGINT DURING EXECUTION, EXITING.");
        exit(ZEN_FAILURE);
    }
}

int main(int argc, char **argv) {
    int code = 0;
    (void)input;
    (void)yyunput;
    signal(SIGSEGV, signalhandler);
    signal(SIGINT, signalhandler);
    if (argc < 2) {
        printf("Copyright (c) 2024 solidracer\nThis project is licensed under the MIT License\nSee LICENSE for details\n\nwritten by solidracer\nzenlang version: %s\nusage: %s <filename>\n", ZEN_VERSION, argv[0]);
        return ZEN_SUCCESS;
    }
    argc--; argv++;
    vm.fname = argv[0];
    zvm_init();
    zc_init();
    yyin = fopen(vm.fname, "r");
    if (!yyin) {
        perror("error");
        return ZEN_FAILURE;
    }
    code = yyparse();

    /*if (!code) zp_dumpbytecode(mchunk.code, mchunk.k);
    return ZEN_SUCCESS;*/

    if (!code) 
        code = zvm_execute(mchunk.code, mchunk.k, mchunk.lines);
    
    zvm_free();
    zc_cleanup();
    fclose(yyin);
    /*printf("\nvm.allocated = %llu\n", vm.allocated);*/
    if (code || vm.allocated)
        fprintf(stderr, "exit code 1 (code = %d, vm.allocated = %llu)\n", code, vm.allocated);
    return code || vm.allocated;
}