LOOP: mov r3,undefinedlabel

mov moreundefinedlaebels, andevenmoreundefinedlabels

.extern unusedlabel

.extern usedlabel

bne usedlabel

.entry definedlabel

definedlabel: mov #5 , r3

jmp imjumpingtothislabel(andimsendingthisone,andthisone)
