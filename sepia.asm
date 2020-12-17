global packed_mul

section .text

; rdi - result data - 4 floats
; rsi - (source data) - 12 floats
; rdx - (constants) - 12 floats
packed_mul:

    movdqu xmm0, [rsi]

    movdqu xmm1, [rsi + 16]
    movdqu xmm2, [rsi + 32]
    movdqu xmm3, [rdx]
    movdqu xmm4, [rdx + 16]
    movdqu xmm5, [rdx + 32]

    mulps xmm0, xmm3
    mulps xmm1, xmm4
    mulps xmm2, xmm5

    addps xmm0, xmm1
    addps xmm0, xmm2

    movdqu [rdi], xmm0
    ret
