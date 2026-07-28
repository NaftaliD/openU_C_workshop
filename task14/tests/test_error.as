mcro brokenMacro
add $0, 1
; Missing mcroend to trigger an error
.entry main
main:
  jmp end
end:
  ret
