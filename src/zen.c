#include "zenCompiler.h"
#include "zenPrint.h"

/*

Copyright (c) 2024 solidracer
zenLang is licensed under the MIT License. See LICENSE for details.

*/

#define ZLMAJOR 1
#define ZLMINOR 0
#define ZLPATCH 0

#define SOURCE_EXT "zl"
#define BYTECODE_EXT "zb"

/* file buffer */
struct file {
    len_t len;
    char *src;
};

static void readfile(const char *fname, struct file *f) {
    f->len = 0;
    f->src = NULL;
    char c;
    FILE *file = fopen(fname, "r");
    if (!file) return;
    while ((c = fgetc(file)) != EOF) {
        f->src = zmem_allocate(f->src, f->len, f->len+1);
        f->src[f->len++] = c;
    }
    f->src = zmem_allocate(f->src, f->len, f->len+1);
    f->src[f->len] = '\0';
    fclose(file);
}

static void freefile(struct file *f) {
    zmem_allocate(f->src, f->len+1, 0);
    f->len = 0;
    f->src = NULL;
}

static void readout(FILE *out, byte **code, value **k, len_t *len, len_t *klen) {
    fread(len, sizeof(len_t), 1, out);
    fread(klen, sizeof(len_t), 1, out);
    *code = ALLOCS(*len);
    *k = ALLOCS(*klen*sizeof(value));
    fread(*code, sizeof(byte), *len, out);
    for (len_t i = 0;i<*klen;i++) {
        value *val = *k + i;
        val->t = fgetc(out);
        switch(val->t) {
            case TYPE_NUMBER: {
                fread(&val->val.n, sizeof(double), 1, out);
                break;
            }
        }
    }
}

static void dohelp(void) {
    printf("zenLang version %d.%d.%d\n", ZLMAJOR, ZLMINOR, ZLPATCH);
    printf("Copyright (c) 2024 solidracer\n");
    printf("zenLang is licensed under the MIT License.\n");
    printf("------------------------\n");
    printf(" ________  ___          \n");
    printf("|\\_____  \\|\\  \\         \n");
    printf(" \\|___/  /\\ \\  \\        \n");
    printf("     /  / /\\ \\  \\      \n");
    printf("    /  /_/__\\ \\  \\____  \n");
    printf("   |\\________\\ \\_______\\\n");
    printf("    \\|_______|\\|_______|\n");
    printf("\n------------------------\nUsage: zen [options]\n");
    printf("  -V           Prints information about the VML state after an execution\n");
    printf("  -f <file>    Specify the file to use (must be a *.zl source or *.zb bytecode)\n");
    printf("  -o <file>    Specify output file to write (will not execute compilation result)\n");
    printf("  -d           Dump the bytecode of the given file\n");
}

static int fileexists(char *fname) {
    FILE *f = fopen(fname, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

typedef enum {
    EXT_SOURCE,
    EXT_BYTECODE,
    EXT_INVALID
} exttype;

static int getext(char *fname) {
    char *ext = strrchr(fname, '.');
    if (ext) {
        ext += 1;
        if (!strcmp(ext, BYTECODE_EXT)) {
            return EXT_BYTECODE;
        }else if (!strcmp(ext, SOURCE_EXT)) {
            return EXT_SOURCE;
        }
    }
    return EXT_INVALID;
}

typedef enum {
    OPT_SUCCESS,
    OPT_INVALID,
    OPT_UNKNOWN,
    OPT_NEEDARG,
} optioncode;

static char *_optarg = NULL;
static char *_curopt = NULL;
/* this is enough for now, will improve later */
static int getoption(int argc, char *argv[], const char *opt, optioncode *code) {
    static int index = 1;
    if (index>=argc) {
        index = 1; /* incase needs to be used again */
        return -1;
    }
    _curopt = argv[index];
    char *f;
    int isopt = strlen(_curopt) == 2 && _curopt[0]=='-';
    if (strlen(_curopt) == 2 && _curopt[0]=='-' && (f = strchr(opt, _curopt[1]))) {
        int needs_arg = *(f+1)==':';
        if (needs_arg) {
            if (index+1 >= argc) {
                *code = OPT_NEEDARG;
                return _curopt[1];
            }
            _optarg = argv[index + 1];
            index += 2;
            *code = OPT_SUCCESS;
            return _curopt[1];
        }else {
            _optarg = NULL;
            index++;
            *code = OPT_SUCCESS;
            return _curopt[1];
        }
    }
    index++;
    if (isopt) { /* an option, but isnt recognized */
        *code = OPT_UNKNOWN;
        return _curopt[1];
    }
    *code = OPT_INVALID;
    return 0;
}

/* flags */

static char *FNAME = NULL;
static char *ONAME = NULL;
static int verbose = 0;
static int debug = 0;

static int handleoptions(int argc, char *args[]) {
    int o;
    optioncode code;
    while ((o = getoption(argc, args, "Vf:o:d", &code)) != -1) {
        switch(code) { /* handle code first */
            case OPT_INVALID: {
                fprintf(stderr, "fatal: '%s' is not an option\n", _curopt);
                if (!strcmp(_curopt, "--help"))
                    fprintf(stderr, "try calling the executable with no arguments for the help menu.\n");
                return 1;
            }
            case OPT_NEEDARG: {
                fprintf(stderr, "fatal: '-%c' needs an argument\n", o);
                return 1;
            }
            case OPT_UNKNOWN: {
                fprintf(stderr, "fatal: '-%c' is unknown\n", o);
                return 1;
            }
            default: break;
        }
        /* now handle the option */
        switch(o) {
            case 'V':
            verbose = 1;
            break;
            case 'f': {
                exttype ext = getext(_optarg);
                if (ext == EXT_INVALID) {
                    fprintf(stderr, "fatal: extension is wrong (expected *.zl or *.zb)\n");
                    return 1;
                }else if (!fileexists(_optarg)) {
                    fprintf(stderr, "fatal: file '%s' could not be found\n", _optarg);
                    return 1;
                }
                FNAME = _optarg;
                break;
            }
            case 'o': {
                exttype ext = getext(_optarg);
                if (ext != EXT_BYTECODE) {
                    fprintf(stderr, "fatal: extension is wrong (expected *.zb)\n");
                    return 1;
                }
                ONAME = _optarg;
                break;
            }
            case 'd': {
                debug = 1;
                break;
            }
        }
    }
    return 0;
}

int main(int argc, char *args[]) {
    #ifdef NOUTBUF
        setvbuf(stdout, NULL, _IONBF, 0);
    #endif
    if (argc==1) {
        dohelp();
        return 0;
    }
    if (handleoptions(argc, args)) return 1;

    if (!FNAME) {
        fprintf(stderr, "fatal: no file given\n");
        return 1;
    }

    if (debug) {
        if (getext(FNAME) != EXT_BYTECODE) {
            fprintf(stderr, "fatal: dumped file must be bytecode (*.zb)");
            return 1;
        }
        zvm_init();
        FILE *f = fopen(FNAME, "rb");
        byte *code;
        value *k;
        len_t len, klen;
        readout(f, &code, &k, &len, &klen);
        zp_printbytecode(code, k, len);
        FREES(code, len);
        REALLOC(k, klen, 0);
        fclose(f);
        zvm_free();
        return 0;
    }

    zvm_init();

    
    if (getext(FNAME) == EXT_BYTECODE) {
        FILE *f = fopen(FNAME, "rb");
        byte *code;
        value *k;
        len_t len, klen;
        readout(f, &code, &k, &len, &klen);
        zvm_execute(code, k);
        FREES(code, len);
        REALLOC(k, klen, 0);
        fclose(f);
    }else {
        struct file main;
        readfile(FNAME, &main);
        zlex_init(main.src);
        int err = zcomp_compile(ONAME);
        if (!err && !ONAME) {
            zvm_execute(comp.code, comp.k);
        }
        freefile(&main);
        FREES(comp.code, comp.len);
        REALLOC(comp.k, comp.klen, 0);
    }
    zvm_free();
    if (verbose) {
        printf("\nvm.allocated: %llu\n", vm.allocated);
        printf("stack pointer offset (vm.sp - vm.stack): %llu\n", vm.sp - vm.stack);
    }
    return 0;
}