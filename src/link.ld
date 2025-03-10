OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv",
       "elf32-littleriscv")
OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY
{
    /* QEMU RV32 virt机器的DRAM从0x80000000开始 */
    DRAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS
{
    .text : {
        *(.text.init)
        *(.text)
    } > DRAM

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > DRAM

    .data : {
        *(.data)
        *(.data.*)
    } > DRAM

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } > DRAM

    . = ALIGN(8);
    _end = .;       /* 定义堆开始的位置 */
    
    /* 堆区 */
    .heap (NOLOAD) : {
        _heap_start = .;
        . = . + 64K;  /* 分配64KB的堆空间 */
        _heap_end = .; /* 定义堆结束的位置 */
    } > DRAM

    /* 栈顶位于DRAM的末尾 */
    _stack_top = ORIGIN(DRAM) + LENGTH(DRAM);
} 