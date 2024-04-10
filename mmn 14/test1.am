bne L3
LENGTH: .data        6               , -9,  15
  MAIN: mov      r3    ,              LENGTH
.extern                  W
    .entry               LENGTH
LOOP: jmp L1(#-1,r6)
prn         #-5
bne W(r4,r5)
sub r1, r4

L1:             inc          K
    .entry          LOOP
bne LOOP(K,W)
STR: .string             "abcdefaaaaaaaaaaaaaaaaaaaaaaaaa"

K: .data             22
    .extern L3
END: stop


 jmp LENGTH

jmp LENGTH(#1,r3)
jmp LENGTH(#1,MAIN)

jmp LENGTH(MAIN,#1)
jmp LENGTH(r3,#1)

jmp LENGTH(r3,MAIN)
jmp LENGTH(r3,r4)



jmp LENGTH(MAIN,r3)

not          r3
not LENGTH

prn #1
prn r3
prn LENGTH

lea   LENGTH  ,            r3
lea                     LENGTH  ,   MAIN

cmp #1  ,  #2047
cmp     #1,  r3
cmp  #1   ,    LENGTH
cmp r3 ,   r4
cmp r3, LENGTH
cmp r3,#1
cmp LENGTH,MAIN
cmp LENGTH,#1
cmp LENGTH,r3

mov #1, r3
mov r3,LENGTH
mov r3,r4
mov LENGTH,r3
mov LENGTH,LOOP

.string "ab   cdefg"
.data 6  , 9 ,  15
