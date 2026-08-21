; error_symbols_macros.as | symbol and macro-definition mistakes Pass One
; should catch: too many operands on a directive, a duplicate label, a
; reserved word as a label, a stray label, and a label reusing a macro name

.entry  COUNTER EXTRA

mcro RESET
        addi    $1,1,$1
mcroend

COUNTER:  add     $1,$2,$3
COUNTER:  sub     $1,$2,$3
add:      hlt
STRAY:
        or      $4,$5,$6
RESET:    hlt
