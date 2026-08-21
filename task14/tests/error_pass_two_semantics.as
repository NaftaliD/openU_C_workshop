; error_pass_two_semantics.as | errors only Pass Two can catch: a jump to
; an undefined symbol, a branch targeting an external symbol, and a
; symbol declared both .entry and .extern

.extern  HELPER
.entry   SHARED

MAIN:   jmp     FINISH
        bne     $1,$2,HELPER
        call    HELPER
.extern SHARED
