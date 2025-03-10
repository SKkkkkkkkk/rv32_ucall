.section .text.init
.global _start
.align 2

_start:
    # 设置栈指针
    la sp, _stack_top
    
    # 跳转到main函数
    call main
    
    # Exit QEMU
    li t0, 0x5555
    li t1, 0x100000
    sw t0, 0(t1)

    j . // should never reach here