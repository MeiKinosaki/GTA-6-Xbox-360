/*
 * Minimal Xenon entry point for the first ELF link test.
 * Hardware and LibXenon subsystem initialization are not performed here.
 */
    .section .text.start,"ax",@progbits
    .globl _start
    .type _start,@function
    .extern gta6_x360_bootstrap

_start:
    bl gta6_x360_bootstrap
1:
    b 1b
    .size _start, .-_start
