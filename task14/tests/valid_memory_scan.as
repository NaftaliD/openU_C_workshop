; valid_memory_scan.as | half-word data plus entry/extern relationships,
; covering or, ori, nori, andi, beq, lh/sh not yet shown in example.as

.extern  PRINTVAL
.entry   SCAN

DATA:   .dh    10, 20, 30, 40, -1
SCAN:   la     DATA
        move   $6,$0
LOOP:   lh     $6,0,$7
        beq    $7,$8,FOUND
        andi   $7,-1,$9
        or     $10,$9,$10
        sh     $10,0,$6
        addi   $6,2,$6
        bne    $7,$0,LOOP
FOUND:  ori    $10,0,$11
        nori   $11,0,$12
        call   PRINTVAL
        hlt
