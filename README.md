# ZENLANG v1.0.0

## BUILDING THE EXECUTABLE
>Bison and Flex are required to compile zenLang!

### STEP 1: RUN THE MAKEFILE

```shell
make parser && make && make clean
```

[See the `MAKE TARGETS` section](#make-targets)

### STEP 2: LOCATE THE EXECUTABLE

The executable is located in the `bin` directory.

### STEP 3: (OPTIONAL BUT RECOMMENDED) EASIER ACCESS

Add the bin directory to the PATH env.

## MAKE TARGETS

- `parser`: **Compiles the parser**
- `clean`:  **Removes parser files (if present), intermediate files (`*.o`)**
- `cleanall`: **Removes parser files (if present), intermediate files (`*.o`), and the final executable**
- `test`: **Run each file in [tests](tests/)**
- `all`: **Compiles all existing `*.c` files into the final executable**

## USAGE

```shell
zen <filename>
```

## VIEWING THE OPTION LIST

Run the executable without options to view the help menu.
```shell
zen
```

## LICENSE

**THIS PROJECT IS LICENSED UNDER THE [MIT LICENSE](LICENSE)** 

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)