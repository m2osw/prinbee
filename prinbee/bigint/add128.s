// add two uint64_t with carry
//
//    // corresponding C function declaration
//    namespace prinbee {
//    void add128(uint64_t * dst, uint64_t const * src);
//    }
//

    .section    .note.GNU-stack,"",@progbits

    .text
    .p2align    4,,15
    .globl      _ZN7prinbee6add128EPmPKm
    .type       _ZN7prinbee6add128EPmPKm, @function
_ZN7prinbee6add128EPmPKm:
    mov         (%rsi), %rax
    add         %rax, (%rdi)
    mov         8(%rsi), %rax
    adc         %rax, 8(%rdi)
    ret

// vim: ts=4 sw=4 et
