# RISC-V Universal Caller Library

A baremetal implementation of a universal function caller for RISC-V RV32G architecture that strictly follows the RISC-V calling convention.

## Overview

This project implements a universal function caller (`rv32_universal_caller()`) for RISC-V 32-bit architecture. The caller can dynamically invoke any function with ilp32/ilp32f/ilp32d calling convention, regardless of its signature, by properly handling argument passing and return values according to the RISC-V calling convention.

## Features

- Supports calling functions with arbitrary signatures
- Supports ilp32/ilp32f/ilp32d calling convention
- Handles all standard C data types (char, short, int, long, long long, float, double, pointers)
- Only supports Scalars for now, no Aggregates(Arrays, Structs, Unions)
- Strictly follows RISC-V calling convention for RV32G architecture
- Properly manages both integer and floating-point registers
- Supports functions with variable number of arguments
- Runs on QEMU RISC-V 32-bit virtual platform

## Project Structure

```
.
├── build/              # Build output directory
├── docs/               # Documentation
│   └── riscv-cc.adoc   # RISC-V calling convention documentation
├── src/                # Source code
│   ├── main.c          # Main program and test cases
│   ├── start.S         # Assembly startup code
│   ├── link.ld         # Linker script
│   ├── universal_caller.c  # Implementation of the universal caller
│   ├── universal_caller.h  # API definitions for the universal caller
│   ├── uart.c          # UART driver for console output
│   ├── uart.h          # UART driver header
│   ├── syscalls.c      # Minimal syscall implementations
│   └── test_funcs.txt  # Test function definitions
├── Makefile            # Build system
└── README.md           # This file
```

## Building the Project

### Prerequisites

- RISC-V GNU Toolchain with RV32G support
- QEMU with RISC-V 32-bit system emulation

### Build Commands

```bash
# Build the project
CROSS_COMPILE=riscv32-unknown-elf- make

# Clean build artifacts
make clean

# Run on QEMU
make run

# Debug with GDB
make debug
```

## Universal Caller API

The universal caller provides a flexible way to call any function with arbitrary arguments:

```c
// Set up arguments
arg_t args[2] = {
    {.type = ARG_INT, .value.i = 42},
    {.type = ARG_FLOAT, .value.f = 3.14f}
};

// Define the function to call
func_t my_func = {
    .func = &target_function,    // Function pointer
    .ret_type = RET_INT,         // Return type
    .arg_count = 2,              // Number of arguments
    .args = args                 // Array of arguments
};

// Call the function and get the return value
return_value_t result = universal_caller(&my_func);
int return_value = result.i;
```

## Debugging

To debug the application:

1. Start QEMU in debug mode: `make debug`
2. In another terminal, start GDB: `riscv32-unknown-elf-gdb build/rv32_hello.elf`
3. Connect to the remote target: `target remote localhost:1234`
4. Load the program: `load`
5. Start debugging: `continue`