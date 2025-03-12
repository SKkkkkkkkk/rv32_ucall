#include "universal_caller.h"
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>

#define XLEN 4 // 32bits = 4 * 8B 

//! (64 * XLEN bits)
#define MAX_STACK_ARGS_SIZE 64


/**
 * 该实现将SP列入clobbers
 * 违反了GCC的以下限制:
 * https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
 * Another restriction is that the clobber list should not contain the stack pointer register. 
 * This is because the compiler requires the value of the stack pointer to be the same after an asm statement as it was on entry to the statement. 
 * However, previous versions of GCC did not enforce this rule and allowed the stack pointer to appear in the list, with unclear semantics. 
 * This behavior is deprecated and listing the stack pointer may become an error in future versions of GCC.
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
return_value_t universal_caller(func_t* func) {
    return_value_t result;
    void *function = func->func;
    int32_t integar_argument_regs[8] = {0};  // a0-a7
    uint32_t integar_argument_regs_index = 0;
    uint32_t stack_args[MAX_STACK_ARGS_SIZE] = {0};
    uint32_t stack_args_index = 0;
    uint32_t stack_args_size_needed = 0;
    
    for (int i = 0; i < func->arg_count; i++) {
        switch (func->args[i].type) {
            case ARG_CHAR:
            case ARG_SHORT:
            case ARG_INT:
            case ARG_LONG:
            case ARG_FLOAT:
            case ARG_POINTER:
                if (integar_argument_regs_index < 8) {
                    integar_argument_regs[integar_argument_regs_index++] = func->args[i].value.i;
                } else {
                    assert(stack_args_index < MAX_STACK_ARGS_SIZE);
                    stack_args[stack_args_index++] = func->args[i].value.i;
                }
                break;
            case ARG_LONG_LONG:
            case ARG_DOUBLE:
                if (integar_argument_regs_index <= 6) {
                    integar_argument_regs[integar_argument_regs_index++] = (uint32_t)func->args[i].value.ll;
                    integar_argument_regs[integar_argument_regs_index++] = (uint32_t)(func->args[i].value.ll >> 32);
                } else if (integar_argument_regs_index == 7) {
                    integar_argument_regs[integar_argument_regs_index++] = (uint32_t)func->args[i].value.ll;
                    assert(stack_args_index < MAX_STACK_ARGS_SIZE);
                    stack_args[stack_args_index++] = (uint32_t)(func->args[i].value.ll >> 32);
                } else {
                    assert(stack_args_index < MAX_STACK_ARGS_SIZE);
                    stack_args[stack_args_index++] = (uint32_t)func->args[i].value.ll;
                    assert(stack_args_index < MAX_STACK_ARGS_SIZE);
                    stack_args[stack_args_index++] = (uint32_t)(func->args[i].value.ll >> 32);
                }
                break;
            default:
                assert(0); // wtf
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
        asm volatile (
            "sub sp, sp, %0\n"
            :: "r" (stack_args_size_needed)
            : "sp", "memory"
        );
        
         // Copy stack arguments to the stack
        asm volatile (
            "mv %0, sp\n"
            : "=r" (sp_addr)
            :
            : "memory"
        );
        for (uint32_t i = 0; i < stack_args_index; i++) {
            sp_addr[i] = stack_args[i];
        }
    }

    // Common function call
    asm volatile (
        // Set up integer arguments (a0-a7)
        "mv a0, %[a0]\n"
        "mv a1, %[a1]\n"
        "mv a2, %[a2]\n"
        "mv a3, %[a3]\n"
        "mv a4, %[a4]\n"
        "mv a5, %[a5]\n"
        "mv a6, %[a6]\n"
        "mv a7, %[a7]\n"
        
        // Call the function
        "jalr ra, %[func], 0\n"
        
        // Capture return values (a0, a1 for all return types in ilp32)
        "mv %[ret_lo], a0\n"
        "mv %[ret_hi], a1\n"
        
        : [ret_lo] "=r" (result._raw32[0]),
          [ret_hi] "=r" (result._raw32[1])
        
        : [func] "r" (function),
          // Integer registers
          [a0] "r" (integar_argument_regs[0]),
          [a1] "r" (integar_argument_regs[1]),
          [a2] "r" (integar_argument_regs[2]),
          [a3] "r" (integar_argument_regs[3]),
          [a4] "r" (integar_argument_regs[4]),
          [a5] "r" (integar_argument_regs[5]),
          [a6] "r" (integar_argument_regs[6]),
          [a7] "r" (integar_argument_regs[7])
        
        // Clobbered registers
        : "ra", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
          "t0", "t1", "t2", "t3", "t4", "t5", "t6", "memory"
    );

    // Restore stack if needed
    if (stack_args_index > 0) {
        asm volatile (
            "add sp, sp, %0\n"
            :: "r" (stack_args_size_needed)
            : "sp", "memory"
        );
    }

    return result;
}
#pragma GCC diagnostic pop