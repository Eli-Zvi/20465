LOOP: bne labeldoesntexist
.entry labelisneverused
.extern labelalreadyexists

jmp label(                     label          , #1)
jsr(label,#1)

mov: mov r3,r4

1label: jmp label(label,label)

.string """""""asdsad""""""""adsadsa"""""""""asda""
.string """"""""""""""""""""""""""""""""""

typing random things

i wonder if im exceeding the character limit on this current line or not what if i am is it going to create an error?

.string what if i just write things after a string dec

.string "what if this string will have a comma at the end of it" ,

mov r8         ,   r3

mov r6, r 3
