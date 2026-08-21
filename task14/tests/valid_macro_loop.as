; valid_macro_loop.as | macro usage plus a countdown loop, covering
; opcodes example.as doesn't: sub, and, nor, subi, blt, lb/sb, mvhi/mvlo

mcro RESET
        sub     $3,$3,$3
        and     $4,$4,$0
mcroend

MAIN:   add     $1,$1,$1
        RESET
        addi    $1,5,$2
LOOP:   subi    $2,1,$2
        lb      $2,0,$5
        nor     $5,$0,$6
        sb      $6,0,$2
        blt     $0,$2,LOOP
        mvhi    $7,$2
        mvlo    $8,$2
DONE:   hlt

.entry  MAIN
