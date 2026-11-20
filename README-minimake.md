# MiniMake

**A lightweight build automation tool written in C, inspired by Unix `make`.**

MiniMake is a systems programming project developed at **EPITA**. Its goal is to reproduce the core behavior of `make`: read a Makefile, resolve dependencies, and execute the commands needed to build a target **only when it is out of date**.

The project focuses on C programming, parsing, dependency management, process execution, and automated testing.

## Features

- **Makefile parsing:** reads targets, dependencies, variables, and tab-indented recipes.
- **Variable expansion:** supports variable references in rules and commands.
- **Dependency resolution:** processes dependencies recursively before building a target.
- **Incremental builds:** compares file modification timestamps to avoid unnecessary rebuilds.
- **Command execution:** runs shell commands using POSIX process management (`fork`, `exec`, and `waitpid`).
- **Error handling:** reports missing targets, invalid input, command failures, and circular dependencies.
- **Command-line options:** supports selecting a Makefile, requesting help, and printing parsed data.
- **Automated tests:** includes a shell-based test suite.

## Requirements

- Linux or another compatible POSIX environment
- A C compiler (such as GCC)
- `make` for building the project and running its tests

## Build and run

From the project's root directory, compile MiniMake:

```bash
make
```

Run it with the default `Makefile`:

```bash
./minimake
```

Build a specific target:

```bash
./minimake target_name
```

Use another Makefile:

```bash
./minimake -f example.mk target_name
```

Other useful options:

```bash
./minimake --help   # Display command-line help
./minimake -p       # Print parsed variables and rules
```

## Example

Create a file named `example.mk`:

```makefile
CC = gcc

hello: hello.c
	$(CC) -o hello hello.c
```

Then run:

```bash
./minimake -f example.mk hello
```

MiniMake checks whether `hello` exists and whether `hello.c` is newer. It executes the compilation command when a rebuild is needed; otherwise, it leaves the up-to-date target untouched.

> **Note:** Recipe commands must begin with a **tab**, as in a standard Makefile.

## How it works

1. **Parse the Makefile:** store variables, rules, dependencies, and recipes in linked data structures.
2. **Select a target:** use the requested target or, by default, the first rule.
3. **Resolve dependencies:** recursively visit prerequisites and detect dependency cycles.
4. **Check timestamps:** determine whether a target is missing or older than its prerequisites.
5. **Run the recipe:** execute its shell commands when rebuilding is necessary and report failures.

## Tests

Run the automated test suite:

```bash
make check
```

The tests exercise the build tool's behavior and help validate parsing, dependency handling, and command execution.

To remove generated build files:

```bash
make clean
```

## Scope

MiniMake is an educational implementation of a **subset** of GNU Make. It is intended to demonstrate the main mechanisms of a build tool, not to provide full GNU Make compatibility or support every advanced Makefile feature.

## Technologies and skills

**C · GCC · Linux/POSIX · Makefiles · Shell scripting · Parsing · Dependency graphs · Process management · Automated testing**

---

*Academic systems programming project, EPITA.*
