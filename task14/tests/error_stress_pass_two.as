; error_stress_pass_two.as -- clean at Pass One, to verify the Pass-Two-
; only error paths (undefined symbol, branch-to-external, entry+extern
; conflict) that error_stress.as's Pass-One errors would otherwise mask.

jmp NEVERDEFINEDANYWHERE
.extern EXTFOO
bne $1,$2,EXTFOO
.entry BOTHENTRYEXTERN
.extern BOTHENTRYEXTERN
