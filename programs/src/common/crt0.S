# CPU start-up: set sp and gp, call main, exit with main's return value.
    .section .text.start
    .globl _start
_start:
    la   sp, __stack_top
    .option push
    .option norelax
    la   gp, __global_pointer$
    .option pop
    call main
    li   a7, 93          # exit
    ecall
1:  j 1b
