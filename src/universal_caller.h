/**
 * universal_caller.h - Header for universal function caller for RISC-V
 *
 * Provides types and functions to call any function with arbitrary signature
 * based on RISC-V calling conventions for rv32g with ilp32 ABI.
 */

#ifndef UNIVERSAL_CALLER_H
#define UNIVERSAL_CALLER_H

#include <stdint.h>
#include <stddef.h>

/**
 * Argument types supported by the universal caller
 */
typedef enum {
    ARG_CHAR,       // 32-bit
    ARG_SHORT,      // 32-bit
    ARG_INT,        // 32-bit
    ARG_LONG,       // 32-bit
    ARG_LONG_LONG,  // 64-bit
    ARG_FLOAT,      // 32-bit
    ARG_DOUBLE,     // 64-bit
    ARG_POINTER     // 32-bit
} arg_type_t;

/**
 * Return types supported by the universal caller
 */
typedef enum {
    RET_VOID,       // No return value
    RET_CHAR,       // 32-bit
    RET_SHORT,      // 32-bit
    RET_INT,        // 32-bit
    RET_LONG,       // 32-bit
    RET_LONG_LONG,  // 64-bit
    RET_FLOAT,      // 32-bit
    RET_DOUBLE,     // 64-bit
    RET_POINTER     // 32-bit
} ret_type_t;

/**
 * Union for passing argument values of different types
 */
typedef union {
    int i;
    long long ll;
    float f;
    double d;
    void *p;
} arg_value_t;

/**
 * Structure representing a single argument with its type and value
 */
typedef struct {
    arg_type_t type;    // Type of the argument
    arg_value_t value;  // Value of the argument
} arg_t;

/**
 * Structure representing a function to be called with all necessary information
 */
typedef struct {
    void *func;         // Function pointer to call
    ret_type_t ret_type;// Return type of the function
    int arg_count;      // Number of arguments
    arg_t *args;        // Array of arguments
} func_t;

/**
 * Call a function described by the func_t structure
 *
 * @param func Pointer to the func_t structure containing function information
 * @return 64-bit value containing the return value
 */
uint64_t universal_call(func_t* func);

#endif /* UNIVERSAL_CALLER_H */ 