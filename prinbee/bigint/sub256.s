// add four uint64_t with carry
//
//    // corresponding C function declaration
//    namespace prinbee {
//    void sub256(uint64_t * dst, uint64_t const * src);
//    }
//

    .macro sub_with_borrow offset
        mov         \offset(%rsi), %rax
        sbb         %rax, \offset(%rdi)
    .endm

    .section    .note.GNU-stack,"",@progbits

    .text
    .p2align    4,,15
    .globl      _ZN7prinbee6sub256EPmPKm
    .type       _ZN7prinbee6sub256EPmPKm, @function
_ZN7prinbee6sub256EPmPKm:
    mov         (%rsi), %rax
    sub         %rax, (%rdi)

    sub_with_borrow 8
    sub_with_borrow 16
    sub_with_borrow 24

    ret

// vim: ts=4 sw=4 et
