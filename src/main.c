#include <stdio.h>
#include <stdint.h>
#include "universal_caller.h"

int32_t test_arg0(void) {
    return 1;
}

int32_t test_add2(int32_t a1, int32_t a2) {
    return a1 + a2;
}

int32_t test_add8(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8) {
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
}

int32_t test_add9(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8, int32_t a9) {
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
}

int32_t test_add32(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8, int32_t a9, int32_t a10, int32_t a11, int32_t a12, int32_t a13, int32_t a14, int32_t a15, int32_t a16, int32_t a17, int32_t a18, int32_t a19, int32_t a20, int32_t a21, int32_t a22, int32_t a23, int32_t a24, int32_t a25, int32_t a26, int32_t a27, int32_t a28, int32_t a29, int32_t a30, int32_t a31, int32_t a32) {
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 + a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 + a31 + a32 ;
}

int64_t test_func0(uint32_t a0, uint64_t a1) {
    return a0 + a1;
}

static void verify_result(int result, int expected) {
    if (result != expected) {
        printf("Test failed: expected %d, got %d\n", expected, result);
    } else {
        printf("Test passed: %d\n", result);
    }
}

void main(void) {
    printf("Hello, RISC-V World!\n");

    func_t func;
    int result32;
    int64_t result64;
    printf("Calling test_arg0()\n");
    func = (func_t){
        .func = test_arg0,
        .ret_type = RET_INT,
        .arg_count = 0,
        .args = NULL
    };
    result32 = universal_call(&func);
    verify_result(result32, 1);

    printf("Calling test_add2(1, 2)\n");
    func = (func_t){
        .func = test_add2,
        .ret_type = RET_INT,
        .arg_count = 2,
        .args = (arg_t[]){{ARG_INT, {.i = 1}}, {ARG_INT, {.i = 2}}}
    };
    result32 = universal_call(&func);
    verify_result(result32, 3);

    printf("Calling test_add8(1, 2, 3, 4, 5, 6, 7, 8)\n");
    func = (func_t){
        .func = test_add8,
        .ret_type = RET_INT,
        .arg_count = 8,
        .args = (arg_t[]){{ARG_INT, {.i = 1}}, {ARG_INT, {.i = 2}}, {ARG_INT, {.i = 3}}, {ARG_INT, {.i = 4}}, {ARG_INT, {.i = 5}}, {ARG_INT, {.i = 6}}, {ARG_INT, {.i = 7}}, {ARG_INT, {.i = 8}}}
    };
    result32 = universal_call(&func);
    verify_result(result32, 36);

    printf("Calling test_add9(1, 2, 3, 4, 5, 6, 7, 8, 9)\n");
    func = (func_t){
        .func = test_add9,
        .ret_type = RET_INT,
        .arg_count = 9,
        .args = (arg_t[]){{ARG_INT, {.i = 1}}, {ARG_INT, {.i = 2}}, {ARG_INT, {.i = 3}}, {ARG_INT, {.i = 4}}, {ARG_INT, {.i = 5}}, {ARG_INT, {.i = 6}}, {ARG_INT, {.i = 7}}, {ARG_INT, {.i = 8}}, {ARG_INT, {.i = 9}}}
    };
    result32 = universal_call(&func);
    verify_result(result32, 45);

    printf("Calling test_add32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32)\n");
    func = (func_t){
        .func = test_add32,
        .ret_type = RET_INT,
        .arg_count = 32,
        .args = (arg_t[]){{ARG_INT, {.i = 1}}, {ARG_INT, {.i = 2}}, {ARG_INT, {.i = 3}}, {ARG_INT, {.i = 4}}, {ARG_INT, {.i = 5}}, {ARG_INT, {.i = 6}}, {ARG_INT, {.i = 7}}, {ARG_INT, {.i = 8}}, {ARG_INT, {.i = 9}}, {ARG_INT, {.i = 10}}, {ARG_INT, {.i = 11}}, {ARG_INT, {.i = 12}}, {ARG_INT, {.i = 13}}, {ARG_INT, {.i = 14}}, {ARG_INT, {.i = 15}}, {ARG_INT, {.i = 16}}, {ARG_INT, {.i = 17}}, {ARG_INT, {.i = 18}}, {ARG_INT, {.i = 19}}, {ARG_INT, {.i = 20}}, {ARG_INT, {.i = 21}}, {ARG_INT, {.i = 22}}, {ARG_INT, {.i = 23}}, {ARG_INT, {.i = 24}}, {ARG_INT, {.i = 25}}, {ARG_INT, {.i = 26}}, {ARG_INT, {.i = 27}}, {ARG_INT, {.i = 28}}, {ARG_INT, {.i = 29}}, {ARG_INT, {.i = 30}}, {ARG_INT, {.i = 31}}, {ARG_INT, {.i = 32}}}
    };
    result32 = universal_call(&func);
    verify_result(result32, 528);

    printf("Calling test_func0(1, 4,294,967,296)\n");
    func = (func_t){
        .func = test_func0,
        .ret_type = RET_LONG_LONG,
        .arg_count = 2,
        .args = (arg_t[]){{ARG_INT, {.i = 1}}, {ARG_LONG_LONG, {.ll = 4294967296}}}
    };
    result64 = universal_call(&func);
    if(result64 != 4294967297) {
        printf("Test failed: expected 4294967297, got %lld\n", result64);
    } else {
        printf("Test passed: %lld\n", result64);
    }
}
