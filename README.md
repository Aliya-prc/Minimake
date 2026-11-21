# MiniMake

MiniMake is a simplified version of the Unix `make` utility, written in C for a systems programming project at EPITA.

The program reads a Makefile, handles dependencies between targets and executes the necessary commands. It checks file modification times to avoid rebuilding files that are already up to date.

## Features

- Parsing of Makefile rules, dependencies, variables and recipes
- Variable expansion
- Recursive dependency management
- File timestamp comparison
- Shell command execution using `fork`, `exec` and `waitpid`
- Detection of circular dependencies and basic error handling

## Compilation and Usage

The project is designed to run on Linux and requires GCC and Make.

Compile with `make`, then run:

- `./minimake` : build the default target
- `./minimake -f file.mk` : use a specific Makefile
- `./minimake -h` : display help
- `./minimake -p` : display parsed rules and variables

## Tests

The project includes a shell test suite covering Makefile parsing, dependencies, command execution and error handling.

Run `make check` to execute the 43 test cases, or `make clean` to remove compilation files.

## Notes

MiniMake implements a subset of GNU Make's functionality and was developed to better understand build automation, dependency management and process execution in C.
