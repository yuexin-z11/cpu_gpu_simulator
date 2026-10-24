/* Static layout for SoCSim programs. 0x00010000-0x00010FFF is reserved
 * for MMIO (launch packet, doorbell, fence). */
ENTRY(_start)

SECTIONS
{
    . = 0x00100000;
    .text   : { *(.text.start) *(.text*) }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*) *(.sdata*) }
    __global_pointer$ = . + 0x800;
    .bss    : { *(.bss*) *(.sbss*) *(COMMON) }
    . = ALIGN(16);
    __heap_start = .;

    __stack_top = 0x00F00000;
}
