#include "universal_caller.h"
#include <stdarg.h>
#include <stdint.h>
#include <malloc.h>

/**
 * Call a function described by the func_t structure
 *
 * @param func Pointer to the func_t structure containing function information
 * @return Union containing the return value in the appropriate type field
 */
return_value_t universal_caller(func_t* func) {
    if (!func) {
        return_value_t zero_result = {0};
        return zero_result;
    }
    
    void *function = func->func;
    return_value_t result = {0};
    union {
        uint64_t u64;
        uint32_t u32[2];
    } raw_result = {0};
    int arg_count = func->arg_count;
    arg_t *args = func->args;
    
    // Count register usage - in ilp32, all arguments use integer registers
    int int_reg_count = 0;
    
    for (int i = 0; i < arg_count; i++) {
        switch (args[i].type) {
            case ARG_CHAR:
            case ARG_SHORT:
            case ARG_INT:
            case ARG_LONG:
            case ARG_FLOAT:
            case ARG_POINTER:
                int_reg_count++;
                break;
            case ARG_LONG_LONG:
            case ARG_DOUBLE:
                // 64-bit values use two integer registers
                int_reg_count += 2;
                break;
        }
    }
    
    // Set up registers and stack
    static uint32_t int_regs[8] = {0};  // a0-a7
    int int_reg_idx = 0;
    
    // Process arguments for registers
    for (int i = 0; i < arg_count && int_reg_idx < 8; i++) {
        switch (args[i].type) {
            case ARG_CHAR:
            case ARG_SHORT:
            case ARG_INT:
            case ARG_LONG:
            case ARG_POINTER: {
                int val = args[i].value.i;
                if (int_reg_idx < 8) {
                    int_regs[int_reg_idx++] = val;
                }
                break;
            }
            
            case ARG_LONG_LONG: {
                long long val = args[i].value.ll;
                if (int_reg_idx <= 6) {  // Need 2 consecutive registers
                    int_regs[int_reg_idx++] = (uint32_t)val;
                    int_regs[int_reg_idx++] = (uint32_t)(val >> 32);
                } else if (int_reg_idx == 7) {
                    // Only one register left, put low bits in register and high bits on stack
                    int_regs[int_reg_idx++] = (uint32_t)val;
                    // High 32 bits will be handled by the stack mechanism
                }
                break;
            }
            
            case ARG_FLOAT: {
                // In ilp32, floats are passed in integer registers
                float val = args[i].value.f;
                
                // Use safer union-based type punning to avoid strict aliasing issues
                union {
                    float f;
                    uint32_t u;
                } converter;
                
                converter.f = val;
                if (int_reg_idx < 8) {
                    int_regs[int_reg_idx++] = converter.u;
                }
                break;
            }
            
            case ARG_DOUBLE: {
                // In ilp32, doubles are passed in pairs of integer registers
                double val = args[i].value.d;
                
                // Use safer union-based type punning to avoid strict aliasing issues
                union {
                    double d;
                    uint64_t u;
                } converter;
                
                converter.d = val;
                uint64_t bits = converter.u;
                
                if (int_reg_idx <= 6) {
                    int_regs[int_reg_idx++] = (uint32_t)bits;
                    int_regs[int_reg_idx++] = (uint32_t)(bits >> 32);
                } else if (int_reg_idx == 7) {
                    // Only one register left, put low bits in register and high bits on stack
                    int_regs[int_reg_idx++] = (uint32_t)bits;
                    // High 32 bits will be handled by the stack mechanism
                }
                break;
            }
        }
    }
    
    // Direct arguments to stack using a static array
    // This avoids having to manipulate the stack directly
    static uint32_t stack_args[64] = {0};
    int stack_idx = 0;
    
    // Fill the stack args array with any arguments that didn't fit in registers
    // or with the high bits of 64-bit values where only one register was left
    for (int i = 0; i < arg_count; i++) {
        // Calculate how many registers this argument would use
        int regs_needed = 0;
        switch (args[i].type) {
            case ARG_CHAR:
            case ARG_SHORT:
            case ARG_INT:
            case ARG_LONG:
            case ARG_FLOAT:
            case ARG_POINTER:
                regs_needed = 1;
                break;
            case ARG_LONG_LONG:
            case ARG_DOUBLE:
                regs_needed = 2;
                break;
        }
        
        // If we would run out of registers, put in stack
        if (int_reg_count <= 8) {
            // All args fit in registers, nothing to do
            break;
        }
        
        // Compute how many registers we've used before this arg
        int regs_used_before = 0;
        for (int j = 0; j < i; j++) {
            switch (args[j].type) {
                case ARG_CHAR:
                case ARG_SHORT:
                case ARG_INT:
                case ARG_LONG:
                case ARG_FLOAT:
                case ARG_POINTER:
                    regs_used_before += 1;
                    break;
                case ARG_LONG_LONG:
                case ARG_DOUBLE:
                    regs_used_before += 2;
                    break;
            }
        }
        
        // If this argument would start at or after register 8, put on stack
        if (regs_used_before >= 8) {
            switch (args[i].type) {
                case ARG_CHAR:
                case ARG_SHORT:
                case ARG_INT:
                case ARG_LONG:
                case ARG_POINTER:
                    stack_args[stack_idx++] = args[i].value.i;
                    break;
                case ARG_FLOAT: {
                    union { float f; uint32_t u; } converter;
                    converter.f = args[i].value.f;
                    stack_args[stack_idx++] = converter.u;
                    break;
                }
                case ARG_LONG_LONG:
                    // Ensure stack alignment for 64-bit values
                    if (stack_idx % 2 != 0) stack_idx++;
                    stack_args[stack_idx++] = (uint32_t)args[i].value.ll;
                    stack_args[stack_idx++] = (uint32_t)(args[i].value.ll >> 32);
                    break;
                case ARG_DOUBLE: {
                    union { double d; uint64_t u; } converter;
                    converter.d = args[i].value.d;
                    // Ensure stack alignment for 64-bit values
                    if (stack_idx % 2 != 0) stack_idx++;
                    stack_args[stack_idx++] = (uint32_t)converter.u;
                    stack_args[stack_idx++] = (uint32_t)(converter.u >> 32);
                    break;
                }
            }
        } 
        // Special case: high 32 bits of a 64-bit value when only one register was left
        else if (regs_used_before == 7 && regs_needed == 2) {
            switch (args[i].type) {
                case ARG_LONG_LONG:
                    stack_args[stack_idx++] = (uint32_t)(args[i].value.ll >> 32);
                    break;
                case ARG_DOUBLE: {
                    union { double d; uint64_t u; } converter;
                    converter.d = args[i].value.d;
                    stack_args[stack_idx++] = (uint32_t)(converter.u >> 32);
                    break;
                }
                default:
                    break;
            }
        }
    }
    
    // Call the function with arguments
    if (stack_idx > 0) {
        // We have stack arguments, need to adjust stack and copy them
        int stack_size = stack_idx * 4;  // Each argument is 4 bytes (32-bit)
        
        // Round up stack size to maintain 16-byte alignment (128-bit)
        stack_size = (stack_size + 15) & ~15;
        
        // First adjust stack - use dynamic size based on arguments
        asm volatile (
            "sub sp, sp, %0\n"
            :: "r" (stack_size)
            : "memory"
        );
        
        // Then copy stack arguments
        uint32_t *sp_addr;
        asm volatile (
            "mv %0, sp\n"
            : "=r" (sp_addr)
            :
            : "memory"
        );
        
        // Copy stack arguments to the stack
        for (int i = 0; i < stack_idx; i++) {
            sp_addr[i] = stack_args[i];
        }
        
        // Now call the function with arguments already on stack
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
            
            // Restore stack with dynamic size
            "add sp, sp, %[stack_size]\n"
            
            // Capture return values (a0, a1 for all return types in ilp32)
            "mv %[ret_lo], a0\n"
            "mv %[ret_hi], a1\n"
            
            : [ret_lo] "=r" (raw_result.u32[0]),
              [ret_hi] "=r" (raw_result.u32[1])
            
            : [func] "r" (function),
              // Integer registers
              [a0] "r" (int_regs[0]),
              [a1] "r" (int_regs[1]),
              [a2] "r" (int_regs[2]),
              [a3] "r" (int_regs[3]),
              [a4] "r" (int_regs[4]),
              [a5] "r" (int_regs[5]),
              [a6] "r" (int_regs[6]),
              [a7] "r" (int_regs[7]),
              [stack_size] "r" (stack_size)
            
            // Clobbered registers
            : "ra", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "memory"
        );
    } else {
        // No stack arguments, simpler call
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
            
            : [ret_lo] "=r" (raw_result.u32[0]),
              [ret_hi] "=r" (raw_result.u32[1])
            
            : [func] "r" (function),
              // Integer registers
              [a0] "r" (int_regs[0]),
              [a1] "r" (int_regs[1]),
              [a2] "r" (int_regs[2]),
              [a3] "r" (int_regs[3]),
              [a4] "r" (int_regs[4]),
              [a5] "r" (int_regs[5]),
              [a6] "r" (int_regs[6]),
              [a7] "r" (int_regs[7])
            
            // Clobbered registers
            : "ra", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "memory"
        );
    }

    // 根据函数的返回类型设置result的适当字段
    result._raw64 = raw_result.u64;
    
    return result;
} 