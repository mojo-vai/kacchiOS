.global ctx_switch

ctx_switch:
    # Save callee-saved registers
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    # Save current stack pointer to first argument (old sp location)
    # Arguments start at esp+20 after 4 pushes (4*4=16 bytes) + return address (4 bytes)
    movl 20(%esp), %eax
    movl %esp, (%eax)

    # Load new stack pointer from second argument
    movl 24(%esp), %esp

    # Restore callee-saved registers
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp

    # Jump to process entry point
    ret