; errors inside the macro body


mcr error

jmp            label     ( #1, r3)

;the next line will be removed by the pre assembler
very very very very very very very very very very very very very very very very long line

endmcr

error

mcr error2

.data --9,,5,,2,4
mov r9, r5

endmcr

mcr error1
;going to duplicate errors

.string "abcdef

jmp label( #1, label)

jmp LOOP(#1,label)  r3

endmcr

error1

error2

mcr error3
endmcr

error3

mcr error4

red
cmp

mov
mov r4

cmp r3

endmcr

error4
