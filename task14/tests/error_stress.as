; error_stress.as -- deliberately malformed content across every parsing
; area Pass One covers, to verify error detection (message + line number)


; --- label definition errors ---
TOOLONGLABELNAMETHATEXCEEDSTHIRTYONECHARS: hlt
1abc: hlt
add: hlt
DUP: hlt
DUP: hlt
LONE:

; --- .db / .dw / .dh value-list errors ---
.db
.db 200
.db -200
.dh 40000
.dh -40000
.dw 99999999999999999999
.dw 5000000000
.db 5,,6
.db ,5,6
.db 5,6,
.db 5 6
.db $LABEL
.db 234f2
.db 5f2

; --- .asciz errors ---
.asciz hello
.asciz "unterminated
.asciz "ok" extra

; --- unknown directive / operation ---
.nosuchdirective 1,2
nosuchop $1,$2,$3

; --- .entry / .extern operand errors ---
.entry
.entry 1abc
.extern add
.entry FOO BAR

; --- register operand errors ---
add 1,$2,$3
add $32,$2,$3
add $-1,$2,$3
add $,$2,$3
add $abc,$2,$3
add $1a,$2,$3

; --- operand separator / count errors ---
add $1 $2,$3
add $1,,$3
add $1,$2,$3,
add ,$1,$2,$3
add $1,$2
add $1,$2,$3,$4

; --- immediate operand errors ---
addi $1,abc,$2
addi $1,99999,$2
addi $1,$2,$3
addi $1,,$2

; --- label-reference operand errors ---
bne $1,$2,1abc
bne $1,$2,add
jmp LOOP,
la
call

; --- jmp-specific errors ---
jmp $99
jmp $-5

; --- hlt errors ---
hlt $1

; --- combined/garbled examples ---
.dw 234f2,,,$LABEL,    ,
add $1,,,$2,,$3
