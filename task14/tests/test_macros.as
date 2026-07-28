; Test file for macro processor
; Tests basic macro definition and expansion

; Define a simple macro
mcro addThree
add $0, 1
add $0, 2
add $0, 3
mcroend

; Define another macro
mcro printValue
lw $1, 0($0)
la $2, strVal
sw $3, 0($2)
mcroend

; Use the macros
addThree
printValue

; Regular assembly code
.entry main
.extern printf

main:
la $0, x
jmp end

end:
ret

; Data section
.db 100, 200, 300
x: .dw 42
.asciz "Value: "
strVal: .dw 0
