// add two uint64_t with carry
//
//    // corresponding C function declaration
//    namespace prinbee {
//    void sub128(uint64_t * dst, uint64_t const * src);
//    }
//

    .section    .note.GNU-stack,"",@progbits

    .text
    .p2align    4,,15
    .globl      _ZN7prinbee6sub128EPmPKm
    .type       _ZN7prinbee6sub128EPmPKm, @function
_ZN7prinbee6sub128EPmPKm:
    mov         (%rsi), %rax
    sub         %rax, (%rdi)
    mov         8(%rsi), %rax
    sbb         %rax, 8(%rdi)
    ret

// vim: ts=4 sw=4 et
