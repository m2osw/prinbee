// subtract multiple uint64_t with borrow
// the returned value is the borrow of the last subtract operation
// so if the function returns 1, there is an overflow
//
//    // corresponding C function declaration
//    namespace prinbee {
//    int sub(uint64_t * a, uint64_t const * b, uint64_t const * c, uint64_t count);
//    }
//

    .text
    .p2align    4,,15
    .globl      _ZN7prinbee3subEPmPKmS2_m
    .type       _ZN7prinbee3subEPmPKmS2_m, @function
_ZN7prinbee3subEPmPKmS2_m:
    clc
    jrcxz       .done
    xor         %r8, %r8

    .p2align    4,,10
    .p2align    3
.loop:
    mov         (%rsi, %r8, 8), %rax
    sbb         (%rdx, %r8, 8), %rax
    mov         %rax, (%rdi, %r8, 8)
    inc         %r8
    loop        .loop

.done:
    setc        %al
    movzx       %al, %rax
    ret

// vim: ts=4 sw=4 et
