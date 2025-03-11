#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include "universal_caller.h"

// Include test functions directly
#include "test_funcs.txt"

// ANSI颜色代码宏定义
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"

/**
 * Print test result and verify against expected value
 */
static void verify_int32(const char* test_name, int32_t result, int32_t expected) {
    if (result == expected) {
        printf(COLOR_GREEN "✓ %s: %ld" COLOR_RESET "\n", test_name, (long)result);
    } else {
        printf(COLOR_RED "✗ %s: expected %ld, got %ld" COLOR_RESET "\n", 
               test_name, (long)expected, (long)result);
    }
}

static void verify_int64(const char* test_name, int64_t result, int64_t expected) {
    if (result == expected) {
        printf(COLOR_GREEN "✓ %s: 0x%llx" COLOR_RESET "\n", test_name, (unsigned long long)result);
    } else {
        printf(COLOR_RED "✗ %s: expected 0x%llx, got 0x%llx" COLOR_RESET "\n", 
               test_name, (unsigned long long)expected, (unsigned long long)result);
    }
}

static void verify_float(const char* test_name, float result, float expected) {
    // Use small epsilon for floating point comparison
    float epsilon = 0.0001f;
    if (result >= expected - epsilon && result <= expected + epsilon) {
        printf(COLOR_GREEN "✓ %s: %f" COLOR_RESET "\n", test_name, result);
    } else {
        printf(COLOR_RED "✗ %s: expected %f, got %f" COLOR_RESET "\n", 
               test_name, expected, result);
    }
}

static void verify_double(const char* test_name, double result, double expected) {
    // Use small epsilon for floating point comparison
    double epsilon = 0.0001;
    if (result >= expected - epsilon && result <= expected + epsilon) {
        printf(COLOR_GREEN "✓ %s: %f" COLOR_RESET "\n", test_name, result);
    } else {
        printf(COLOR_RED "✗ %s: expected %f, got %f" COLOR_RESET "\n", 
               test_name, expected, result);
    }
}

static int32_t helper_add(int32_t a, int32_t b) {
    return a + b;
}

// Main function to test all cases
void main(void) {
    printf("=== Testing rv32_universal_caller ===\n\n");
    
    func_t func;
    return_value_t result;
    
    // Test 1: No arguments
    printf("Test 1: No arguments\n");
    func = (func_t){
        .func = test_no_args,
        .ret_type = RET_INT,
        .arg_count = 0,
        .args = NULL
    };
    result = universal_call(&func);
    verify_int32("test_no_args", result.i, 42);
    
    // Test 2: Various return types
    printf("\nTest 2: Various return types\n");
    
    // Int32 return
    func = (func_t){
        .func = test_return_int32,
        .ret_type = RET_INT,
        .arg_count = 0,
        .args = NULL
    };
    result = universal_call(&func);
    verify_int32("test_return_int32", result.i, 1);
    
    // Int64 return
    func = (func_t){
        .func = test_return_int64,
        .ret_type = RET_LONG_LONG,
        .arg_count = 0,
        .args = NULL
    };
    result = universal_call(&func);
    verify_int64("test_return_int64", result.ll, 0x0123456789ABCDEF);
    
    // Float return
    func = (func_t){
        .func = test_return_float,
        .ret_type = RET_FLOAT,
        .arg_count = 0,
        .args = NULL
    };
    result = universal_call(&func);
    verify_float("test_return_float", result.f, 3.14f);
    
    // Double return
    func = (func_t){
        .func = test_return_double,
        .ret_type = RET_DOUBLE,
        .arg_count = 0,
        .args = NULL
    };
    result = universal_call(&func);
    verify_double("test_return_double", result.d, 2.71828);
    
    // Void return
    func = (func_t){
        .func = test_return_void,
        .ret_type = RET_VOID,
        .arg_count = 0,
        .args = NULL
    };
    universal_call(&func);
    printf(COLOR_GREEN "✓ test_return_void called successfully" COLOR_RESET "\n");
    
    // Test 3: Register arguments (8 arguments)
    printf("\nTest 3: Register arguments (8 arguments)\n");
    func = (func_t){
        .func = test_reg_args,
        .ret_type = RET_INT,
        .arg_count = 8,
        .args = (arg_t[]){
            {ARG_INT, {.i = 1}},
            {ARG_INT, {.i = 2}},
            {ARG_INT, {.i = 3}},
            {ARG_INT, {.i = 4}},
            {ARG_INT, {.i = 5}},
            {ARG_INT, {.i = 6}},
            {ARG_INT, {.i = 7}},
            {ARG_INT, {.i = 8}}
        }
    };
    result = universal_call(&func);
    verify_int32("test_reg_args", result.i, 36); // 1+2+3+4+5+6+7+8 = 36
    
    // Test 4: Stack arguments (>8 arguments)
    printf("\nTest 4: Stack arguments (>8 arguments)\n");
    func = (func_t){
        .func = test_stack_args,
        .ret_type = RET_INT,
        .arg_count = 10,
        .args = (arg_t[]){
            {ARG_INT, {.i = 1}},
            {ARG_INT, {.i = 2}},
            {ARG_INT, {.i = 3}},
            {ARG_INT, {.i = 4}},
            {ARG_INT, {.i = 5}},
            {ARG_INT, {.i = 6}},
            {ARG_INT, {.i = 7}},
            {ARG_INT, {.i = 8}},
            {ARG_INT, {.i = 9}},
            {ARG_INT, {.i = 10}}
        }
    };
    result = universal_call(&func);
    verify_int32("test_stack_args", result.i, 55); // 1+2+3+...+10 = 55
    
    // Test 5: Mixed argument types
    printf("\nTest 5: Mixed argument types\n");
    func = (func_t){
        .func = test_mixed_types,
        .ret_type = RET_DOUBLE,
        .arg_count = 7,
        .args = (arg_t[]){
            {ARG_CHAR, {.i = 100}},             // int8_t
            {ARG_SHORT, {.i = 2000}},           // int16_t
            {ARG_INT, {.i = 30000}},            // int32_t
            {ARG_LONG_LONG, {.ll = 400000LL}},  // int64_t
            {ARG_FLOAT, {.f = 5.5f}},           // float
            {ARG_DOUBLE, {.d = 6.6}},           // double
            {ARG_POINTER, {.p = (void*)7}}      // void*
        }
    };
    result = universal_call(&func);
    verify_double("test_mixed_types", result.d, (int8_t)100 + (int16_t)2000 + (int32_t)30000 + (int64_t)400000 + 5.5f + 6.6 + (intptr_t)(void*)7);
    
    // Test 6: Floating point arguments
    printf("\nTest 6: Floating point arguments\n");
    func = (func_t){
        .func = test_float_args,
        .ret_type = RET_DOUBLE,
        .arg_count = 4,
        .args = (arg_t[]){
            {ARG_FLOAT, {.f = 1.1f}},
            {ARG_FLOAT, {.f = 2.2f}},
            {ARG_DOUBLE, {.d = 3.3}},
            {ARG_DOUBLE, {.d = 4.4}}
        }
    };
    result = universal_call(&func);
    verify_double("test_float_args", result.d, 11.0);
    
    // Test 7: Many arguments
    printf("\nTest 7: Many arguments (20 args)\n");
    func = (func_t){
        .func = test_many_args,
        .ret_type = RET_INT,
        .arg_count = 20,
        .args = (arg_t[]){
            {ARG_INT, {.i = 1}},
            {ARG_INT, {.i = 2}},
            {ARG_INT, {.i = 3}},
            {ARG_INT, {.i = 4}},
            {ARG_INT, {.i = 5}},
            {ARG_INT, {.i = 6}},
            {ARG_INT, {.i = 7}},
            {ARG_INT, {.i = 8}},
            {ARG_INT, {.i = 9}},
            {ARG_INT, {.i = 10}},
            {ARG_INT, {.i = 11}},
            {ARG_INT, {.i = 12}},
            {ARG_INT, {.i = 13}},
            {ARG_INT, {.i = 14}},
            {ARG_INT, {.i = 15}},
            {ARG_INT, {.i = 16}},
            {ARG_INT, {.i = 17}},
            {ARG_INT, {.i = 18}},
            {ARG_INT, {.i = 19}},
            {ARG_INT, {.i = 20}}
        }
    };
    result = universal_call(&func);
    verify_int32("test_many_args", result.i, 210); // Sum of 1 to 20 = 210
    
    // Test 8: Boundary values
    printf("\nTest 8: Boundary values\n");
    func = (func_t){
        .func = test_boundary_values,
        .ret_type = RET_INT,
        .arg_count = 3,
        .args = (arg_t[]){
            {ARG_INT, {.i = INT32_MIN}},
            {ARG_INT, {.i = INT32_MAX}},
            {ARG_LONG_LONG, {.ll = 0x1234567887654321LL}}
        }
    };
    result = universal_call(&func);
    verify_int32("test_boundary_values", result.i, INT32_MIN + INT32_MAX + (int32_t)0x1234567887654321LL);
    
    // Test 9: Function pointer
    printf("\nTest 9: Function pointer\n");
    func = (func_t){
        .func = test_function_pointer,
        .ret_type = RET_INT,
        .arg_count = 3,
        .args = (arg_t[]){
            {ARG_POINTER, {.p = helper_add}},
            {ARG_INT, {.i = 123}},
            {ARG_INT, {.i = 456}}
        }
    };
    result = universal_call(&func);
    verify_int32("test_function_pointer", result.i, 579); // 123 + 456 = 579
    
    printf("\n=== All tests completed ===\n");
}
