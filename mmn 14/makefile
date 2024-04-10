assembler:  assembler.o pre_assembler.o first_transition.o second_transition.o
		gcc  -g -Wall -ansi -pedantic assembler.o pre_assembler.o first_transition.o second_transition.o -lm -o assembler
		
assembler.o: assembler.c pre_assembler.h first_transition.h
		gcc -c -Wall -ansi -pedantic assembler.c -o assembler.o
		
pre_assembler.o: pre_assembler.c pre_assembler.h 
		gcc -c -Wall -ansi -pedantic pre_assembler.c -o pre_assembler.o	
		
first_transition.o: first_transition.c first_transition.h second_transition.h data_structures.h 
		gcc -c -Wall -ansi -pedantic first_transition.c -o first_transition.o
		
second_transition.o: second_transition.c data_structures.h second_transition.h
		gcc -c -Wall -ansi -pedantic second_transition.c -o second_transition.o
