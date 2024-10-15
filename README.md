# zen v2.0.0 RELEASE NOTES

**Previous version: zen v1.0.1**

- Fixed a bug where the lexer buffer would not be freed correctly.
- Introduced `do while` loops. [example](tests/dowhile.zl)
- Introduced `match` statements. [example](tests/match.zl)
- Introduced `global` variables. [example](tests/globals.zl)
- Introduced the `delete` keyword (for globals). [example](tests/delete.zl)
- Introduced the '!' unary operator.
- Tables are added as a data type. [example](tests/hashtables.zl)
- Fixed minor precedence issues.

## BUILDING THE EXECUTABLE
>**Bison** and **Flex** are required to compile zenLang!

### STEP 1: RUN THE MAKEFILE

```shell
make parser && make && make clean
```

[See the `MAKE TARGETS` section.](#make-targets)

### STEP 2: LOCATE THE EXECUTABLE

The executable is located in the `bin` directory.

### STEP 3: (OPTIONAL BUT RECOMMENDED) ADD TO PATH

Add the bin directory to the PATH env.

## MAKE TARGETS

- `parser`: **Compiles the parser**
- `clean`:  **Removes parser files (if present), intermediate files (`*.o`)**
- `cleanall`: **Removes parser files (if present), intermediate files (`*.o`), and the final executable**
- `test`: **Run each file in [tests](tests/)**
- `all`: **Compiles all existing `*.c` files into the final executable**

## USAGE

```shell
zen [options] <filename>
```

## VIEWING THE OPTION LIST

Run the executable with the option `-h`.
```shell
zen -h
```

## LICENSE

**THIS PROJECT IS LICENSED UNDER THE [MIT LICENSE](LICENSE).** 

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)