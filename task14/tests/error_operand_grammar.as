; error_operand_grammar.as | a small running-total routine with a few
; grammar slips that Pass One should catch

MAIN:   ad      $1,$2,$3
        add     $32,$1,$3
        addi    $3,99999,$3
        sub     $3 $1,$3
TOTAL:  hlt     $3
