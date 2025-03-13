#include "universal_caller.h"
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#if __riscv_float_abi_soft == 1
#pragma message("ilp32")
#elif __riscv_float_abi_single == 1
#pragma message("ilp32f")
#elif __riscv_float_abi_double == 1
#pragma message("ilp32d")
#else
#error "unknown float abi"
#endif

#define XLEN 4 // 32bits = 4 * 8B = 32B

//! (64 * XLEN bits)
#define MAX_STACK_ARGS_SIZE 64

typedef union {
  float f;
  double d;
  uint32_t raw32[2];
  uint64_t raw64;
} fp_reg_t;

/**
 * 该实现将SP列入clobbers
 * 违反了GCC的以下限制:
 * https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
 * Another restriction is that the clobber list should not contain the stack
 * pointer register. This is because the compiler requires the value of the
 * stack pointer to be the same after an asm statement as it was on entry to the
 * statement. However, previous versions of GCC did not enforce this rule and
 * allowed the stack pointer to appear in the list, with unclear semantics. This
 * behavior is deprecated and listing the stack pointer may become an error in
 * future versions of GCC.
 * 但是调试发现gcc 13.2.0版本实际上尊重了sp加入clobbers的提示，并且采取了相应的安全措施刚好满足需求(stack变量变为s0从上到下检索，而非通过sp从下到上检索，因为我们会改变sp)
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
/**
 * Call a function described by the func_t structure
 *
 * @param func Pointer to the func_t structure containing function information
 * @return Union containing the return value in the appropriate type field
 */
return_value_t universal_caller(func_t *func) {
  return_value_t result;
  void *function = func->func;
  int32_t integar_argument_regs[8] = {0}; // a0-a7
  uint32_t integar_argument_regs_index = 0;
#if __riscv_float_abi_soft != 1
  return_value_t result_fp;
  fp_reg_t fp_argument_regs[8] = {0}; // f0-f7
  uint32_t fp_argument_regs_index = 0;
#endif
  uint32_t stack_args[MAX_STACK_ARGS_SIZE] = {0};
  uint32_t stack_args_index = 0;
  uint32_t stack_args_size_needed = 0;

#define HANDLE_INTEGER_CALLING_CONVENTION_1XLEN                                \
  INTEGER_CALLING_CONVENTION_1XLEN:                                            \
  if (integar_argument_regs_index < 8) {                                       \
    integar_argument_regs[integar_argument_regs_index++] =                     \
        func->args[i].value.i;                                                 \
  } else {                                                                     \
    assert(stack_args_index < MAX_STACK_ARGS_SIZE);                            \
    stack_args[stack_args_index++] = func->args[i].value.i;                    \
  }
#define HANDLE_INTEGER_CALLING_CONVENTION_2XLEN                                \
  INTEGER_CALLING_CONVENTION_2XLEN:                                            \
  if (integar_argument_regs_index <= 6) {                                      \
    integar_argument_regs[integar_argument_regs_index++] =                     \
        (uint32_t)func->args[i].value.ll;                                      \
    integar_argument_regs[integar_argument_regs_index++] =                     \
        (uint32_t)(func->args[i].value.ll >> 32);                              \
  } else if (integar_argument_regs_index == 7) {                               \
    integar_argument_regs[integar_argument_regs_index++] =                     \
        (uint32_t)func->args[i].value.ll;                                      \
    assert(stack_args_index < MAX_STACK_ARGS_SIZE);                            \
    stack_args[stack_args_index++] = (uint32_t)(func->args[i].value.ll >> 32); \
  } else {                                                                     \
    stack_args_index = ((stack_args_index + 1) &                               \
                        ~1); /* address needs to be aligned to 2XLEN */        \
    assert(stack_args_index < MAX_STACK_ARGS_SIZE);                            \
    stack_args[stack_args_index++] = (uint32_t)func->args[i].value.ll;         \
    assert(stack_args_index < MAX_STACK_ARGS_SIZE);                            \
    stack_args[stack_args_index++] = (uint32_t)(func->args[i].value.ll >> 32); \
  }

  for (int i = 0; i < func->arg_count; i++) {
    switch (func->args[i].type) {
    // Integer
    case ARG_CHAR:
    case ARG_SHORT:
    case ARG_INT:
    case ARG_LONG:
    case ARG_POINTER:
      HANDLE_INTEGER_CALLING_CONVENTION_1XLEN
      break;
    case ARG_LONG_LONG:
      HANDLE_INTEGER_CALLING_CONVENTION_2XLEN
      break;
      // Floating-point
#if __riscv_float_abi_soft == 1
    case ARG_FLOAT:
      goto INTEGER_CALLING_CONVENTION_1XLEN;
    case ARG_DOUBLE:
      goto INTEGER_CALLING_CONVENTION_2XLEN;
#elif __riscv_float_abi_single == 1
    case ARG_FLOAT:
      if (fp_argument_regs_index < 8) {
        fp_argument_regs[fp_argument_regs_index++].f = func->args[i].value.f;
        break;
      }
      goto INTEGER_CALLING_CONVENTION_1XLEN;
    case ARG_DOUBLE:
      goto INTEGER_CALLING_CONVENTION_2XLEN;
#elif __riscv_float_abi_double == 1
    case ARG_FLOAT:
      if (fp_argument_regs_index < 8) {
        fp_argument_regs[fp_argument_regs_index].f = func->args[i].value.f;
        fp_argument_regs[fp_argument_regs_index].raw32[1] =
            0xFFFFFFFF; // 1-extended (NaN-boxed) to FLEN bits
        fp_argument_regs_index++;
        break;
      }
      goto INTEGER_CALLING_CONVENTION_1XLEN;
    case ARG_DOUBLE:
      if (fp_argument_regs_index < 8) {
        fp_argument_regs[fp_argument_regs_index++].d = func->args[i].value.d;
        break;
      }
      goto INTEGER_CALLING_CONVENTION_2XLEN;
#else
#error "unknown abi"
#endif
    default:
      assert(0); // unknown argument type
    }
  }

  if (stack_args_index > 0) {
    stack_args_size_needed = ((stack_args_index * XLEN) + 15) & ~15;
  }
  assert(integar_argument_regs_index <= 8);
  assert(stack_args_size_needed <= (MAX_STACK_ARGS_SIZE * XLEN));

  // Prepare stack arguments if needed
  uint32_t *sp_addr = NULL;
  if (stack_args_index > 0) {
    // Adjust stack - use dynamic size based on arguments
    asm volatile("sub sp, sp, %0\n" ::"r"(stack_args_size_needed)
                 : "sp", "memory");

    // Copy stack arguments to the stack
    asm volatile("mv %0, sp\n" : "=r"(sp_addr) : : "memory");
    for (uint32_t i = 0; i < stack_args_index; i++) {
      sp_addr[i] = stack_args[i];
    }
  }

  // Common function call
  asm volatile(
      // Set up integer arguments (a0-a7)
      "lw a0, %[a0]\n"
      "lw a1, %[a1]\n"
      "lw a2, %[a2]\n"
      "lw a3, %[a3]\n"
      "lw a4, %[a4]\n"
      "lw a5, %[a5]\n"
      "lw a6, %[a6]\n"
      "lw a7, %[a7]\n"

#if __riscv_float_abi_single == 1
      "flw fa0, %[fa0]\n"
      "flw fa1, %[fa1]\n"
      "flw fa2, %[fa2]\n"
      "flw fa3, %[fa3]\n"
      "flw fa4, %[fa4]\n"
      "flw fa5, %[fa5]\n"
      "flw fa6, %[fa6]\n"
      "flw fa7, %[fa7]\n"
#elif __riscv_float_abi_double == 1
      "fld fa0, %[fa0]\n"
      "fld fa1, %[fa1]\n"
      "fld fa2, %[fa2]\n"
      "fld fa3, %[fa3]\n"
      "fld fa4, %[fa4]\n"
      "fld fa5, %[fa5]\n"
      "fld fa6, %[fa6]\n"
      "fld fa7, %[fa7]\n"
#endif

      // Call the function
      "jalr ra, %[func], 0\n"

      // Capture return values (a0, a1 for integer, fa0 for float/double)
      "sw a0, %[ret_lo]\n"
      "sw a1, %[ret_hi]\n"
#if __riscv_float_abi_single == 1
      "fsw fa0, %[ret_fp]\n"
#elif __riscv_float_abi_double == 1
      "fsd fa0, %[ret_fp]\n"
#endif
      : [ret_lo] "=m"(result._raw32[0]), [ret_hi] "=m"(result._raw32[1])
#if __riscv_float_abi_single == 1
                                             ,
        [ret_fp] "=m"(result_fp.f)
#elif __riscv_float_abi_double == 1
                                             ,
        [ret_fp] "=m"(result_fp.d)
#endif

      : [func] "r"(function),
        // Integer registers
        [a0] "m"(integar_argument_regs[0]), [a1] "m"(integar_argument_regs[1]),
        [a2] "m"(integar_argument_regs[2]), [a3] "m"(integar_argument_regs[3]),
        [a4] "m"(integar_argument_regs[4]), [a5] "m"(integar_argument_regs[5]),
        [a6] "m"(integar_argument_regs[6]), [a7] "m"(integar_argument_regs[7])
#if __riscv_float_abi_single == 1
                                                ,
        [fa0] "m"(fp_argument_regs[0].f), [fa1] "m"(fp_argument_regs[1].f),
        [fa2] "m"(fp_argument_regs[2].f), [fa3] "m"(fp_argument_regs[3].f),
        [fa4] "m"(fp_argument_regs[4].f), [fa5] "m"(fp_argument_regs[5].f),
        [fa6] "m"(fp_argument_regs[6].f), [fa7] "m"(fp_argument_regs[7].f)
#elif __riscv_float_abi_double == 1
                                                ,
        [fa0] "m"(fp_argument_regs[0].d), [fa1] "m"(fp_argument_regs[1].d),
        [fa2] "m"(fp_argument_regs[2].d), [fa3] "m"(fp_argument_regs[3].d),
        [fa4] "m"(fp_argument_regs[4].d), [fa5] "m"(fp_argument_regs[5].d),
        [fa6] "m"(fp_argument_regs[6].d), [fa7] "m"(fp_argument_regs[7].d)
#endif
      // Clobbered registers
      : "ra", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "t0", "t1", "t2",
        "t3", "t4", "t5", "t6",
#if __riscv_float_abi_single != 1
        "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "ft0", "ft1",
        "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "ft8", "ft9", "ft10", "ft11",
#endif
        "memory");

  // Restore stack if needed
  if (stack_args_index > 0) {
    asm volatile("add sp, sp, %0\n" ::"r"(stack_args_size_needed)
                 : "sp", "memory");
  }

#if __riscv_float_abi_soft == 1
  return result;
#elif __riscv_float_abi_single == 1
  return (func->ret_type == RET_FLOAT) ? result_fp : result;
#elif __riscv_float_abi_double == 1
  return ((func->ret_type == RET_DOUBLE) || (func->ret_type == RET_FLOAT))
             ? result_fp
             : result;
#else
#error "unknown abi"
#endif
}
#pragma GCC diagnostic pop