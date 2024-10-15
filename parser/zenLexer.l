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
match {return MATCH;}
case {return CASE;}
default {return DEFAULT;}
false {return TFALSE;}
print {return PRINT;}
var {return VAR;}
while {return WHILE;}
break {return BREAK;}
global {return GLOBAL;}
delete {return DELETE;}
const {return CONST;}
do {return DO;}
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
    freebuf();
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
        fprintf(stderr, "interrupt signal recieved\n");
        exit(ZEN_FAILURE);
    }
}

static char *options = "advh";
static int code, _dump, _abort;
static char *_n;

int optf(char o, char *arg) {
    (void)arg;
    switch (o) {
        case 'a': _abort = 1; return 0;
        case 'd': _dump = 1; return 0;
        case 'h': {
            printf("Copyright (c) 2024 solidracer\nThis project is licensed under the MIT License\nSee LICENSE for details\n\nwritten by solidracer\nzenlang version: %s\n\nUsage:\n\t%s [options] <filename>\n\nOptions:\n\t-d\tdump compiled bytecode\n\t-v\tprint version\n\t-a\tabort before executing the bytecode stream\n\t-h\tview this help menu\n", ZEN_VERSION, _n);
            return 1;
        }
        case 'v': printf("zenlang v%s\n", ZEN_VERSION); return 0;
        default: return 1;
    }
}

int main(int argc, char **argv) {
    int i;
    _n = argv[0];
    (void)input;
    (void)yyunput;
    signal(SIGSEGV, signalhandler);
    signal(SIGINT, signalhandler);
    if (argc < 2) {
        fprintf(stderr, "\nerror (options): no options given\n\n");
        optf('h', NULL);
        return ZEN_FAILURE;
    }
    argc--; argv++;
    vm.fname = NULL;
    for (i = 0;i<argc;i++) {
        char *arg = argv[i];
        if (arg[0]=='-') {
            arg++;
            int len = strlen(arg);
            int j;
            for (j = 0;j<len;j++) {
                char option = arg[j];
                char *x = strchr(options, option);
                if (!x) {
                    fprintf(stderr, "error (options): Unrecognized option\n");
                    return ZEN_FAILURE;
                }
                /* only the last option can get an argument */
                if (*(x+1) == ':' && (i+1 >= argc || j != len-1)) {
                    fprintf(stderr, "error (options): Need argument for '-%c'\n", option);
                    return ZEN_FAILURE;
                }
                if (optf(option, (*(x+1)==':' && j == len-1)?argv[i++]:NULL)) return ZEN_SUCCESS;
            }
        }else {
            if (!vm.fname) vm.fname = arg; else {
                fprintf(stderr, "error (options): Filename already set\n");
                return ZEN_FAILURE;
            }
        }
    }
    zvm_init();
    zc_init();
    yyin = fopen(vm.fname, "r");
    if (!yyin) {
        perror("error (filename)");
        return ZEN_FAILURE;
    }
    code = yyparse();

    if (!code && _dump) zp_dumpbytecode(mchunk.code, mchunk.k);

    if (_abort) goto a;

    if (!code) 
        code = zvm_execute(mchunk.code, mchunk.k, mchunk.lines);
    
    a:
    zvm_free();
    zc_cleanup();
    fclose(yyin);
    /*printf("\nvm.allocated = %llu\n", vm.allocated);*/
    if (code || vm.allocated || vm.sp - vm.stack)
        fprintf(stderr, "exit code 1 (code = %d, vm.allocated = %llu, vm.sp - vm.stack = %llu)\n", code, vm.allocated, vm.sp - vm.stack);
    return code || vm.allocated || vm.sp - vm.stack;
}