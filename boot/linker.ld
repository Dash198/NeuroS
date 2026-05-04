OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{

    /* Move the memory cursor down to RAM start. */
    . = 0x80000000;

    /* Next we place all the important sections. */

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss .bss.*)
    }

    /* Move the cursor by 8KB to allocate memory for the stack. */
    . += 8192;

    /* Define the stack top. */
    _stack_top = .;
}
