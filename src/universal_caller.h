/**
 * universal_caller.h - Header for universal function caller for RISC-V
 *
 * Provides types and functions to call any function with arbitrary signature
 * based on RISC-V calling conventions for rv32g with ilp32 ABI.
 */

#ifndef UNIVERSAL_CALLER_H
#define UNIVERSAL_CALLER_H

#include <stddef.h>
#include <stdint.h>

_Static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__,
               "Host must be little-endian!");

/**
 * Argument types supported by the universal caller
 */
typedef enum : uint32_t {
  ARG_CHAR,      // 32-bit
  ARG_SHORT,     // 32-bit
  ARG_INT,       // 32-bit
  ARG_LONG,      // 32-bit
  ARG_LONG_LONG, // 64-bit
  ARG_FLOAT,     // 32-bit
  ARG_DOUBLE,    // 64-bit
  ARG_POINTER    // 32-bit
} arg_type_t;

/**
 * Return types supported by the universal caller
 */
typedef enum : uint32_t {
  RET_VOID,      // No return value
  RET_CHAR,      // 32-bit
  RET_SHORT,     // 32-bit
  RET_INT,       // 32-bit
  RET_LONG,      // 32-bit
  RET_LONG_LONG, // 64-bit
  RET_FLOAT,     // 32-bit
  RET_DOUBLE,    // 64-bit
  RET_POINTER    // 32-bit
} ret_type_t;

/**
 * Union representing function return values of different types
 */
typedef union {
  char c;
  short s;
  int i;
  long l;
  long long ll;
  float f;
  double d;
  void *p;
  uint32_t _raw32[2]; // Raw 32-bit value (for internal use)
} return_value_t;

/**
 * Union for passing argument values of different types
 */
typedef union {
  int32_t c;  // char
  int32_t s;  // short
  int32_t i;  // int
  int32_t l;  // long
  int64_t ll; // long long
  float f;    // float
  double d;   // double
#if (__riscv == 1) && (__riscv_xlen == 32)
  void *p; // pointer
#else
  uint32_t p;
#endif
} arg_value_t;
_Static_assert(sizeof(arg_value_t) == 8, "arg_value_t 大小必须为 8 字节");
_Static_assert(sizeof(float) == 4, "float 大小必须为 4 字节");
_Static_assert(sizeof(double) == 8, "double 大小必须为 8 字节");

/**
 * Structure representing a single argument with its type and value
 */
typedef struct {
  arg_type_t type;   // Type of the argument
  arg_value_t value; // Value of the argument
} arg_t;
_Static_assert(sizeof(arg_t) == 16, "arg_t 大小必须为 16 字节");
_Static_assert(offsetof(arg_t, type) == 0, "arg_t.type 偏移错误");
_Static_assert(offsetof(arg_t, value) == 8, "arg_t.value 偏移错误");

/**
 * Structure representing a function to be called with all necessary information
 */
typedef struct {
#if (__riscv == 1) && (__riscv_xlen == 32)
  void *func; // Function pointer to call
#else
  uint32_t func;
#endif
  ret_type_t ret_type; // Return type of the function
  int32_t arg_count;   // Number of arguments
#if (__riscv == 1) && (__riscv_xlen == 32)
  arg_t *args; // Array of arguments
#else
  uint32_t args; // Array of arguments
#endif
} func_t;
_Static_assert(sizeof(func_t) == 16, "func_t 大小必须为 16 字节");
_Static_assert(offsetof(func_t, func) == 0, "func_t.func 偏移错误");
_Static_assert(offsetof(func_t, ret_type) == 4, "func_t.ret_type 偏移错误");
_Static_assert(offsetof(func_t, arg_count) == 8, "func_t.arg_count 偏移错误");
_Static_assert(offsetof(func_t, args) == 12, "func_t.args 偏移错误");

/**
 * Call a function described by the func_t structure
 *
 * @param func Pointer to the func_t structure containing function information
 * @return Union containing the return value in the appropriate type field
 */
return_value_t universal_caller(func_t *func);

#endif /* UNIVERSAL_CALLER_H */