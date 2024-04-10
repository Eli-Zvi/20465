;this file will test macros

mcr m1

mov r4,r3

mov r5 ,    r6

mov label,       r3



.string "abcdef"


;comment in a macro 

jmp LOOP(r4,#1)

endmcr


.entry label

.extern LOOP

label: mov LOOP,r3



m1

mcr m2

MAIN:    bne label

cmp r3,r4

not r7


.data 3,6 ,    4     ,    9

endmcr
