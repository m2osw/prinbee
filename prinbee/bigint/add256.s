// add four uint64_t with carry
//
//    // corresponding C function declaration
//    namespace prinbee {
//    void add256(uint64_t * dst, uint64_t const * src);
//    }
//

    .macro add_with_carry offset
        mov         \offset(%rsi), %rax
        adc         %rax, \offset(%rdi)
    .endm

    .section    .note.GNU-stack,"",@progbits

    .text
    .p2align    4,,15
    .globl      _ZN7prinbee6add256EPmPKm
    .type       _ZN7prinbee6add256EPmPKm, @function
_ZN7prinbee6add256EPmPKm:
    mov         (%rsi), %rax
    add         %rax, (%rdi)

    add_with_carry 8
    add_with_carry 16
    add_with_carry 24

    ret

// vim: ts=4 sw=4 et
