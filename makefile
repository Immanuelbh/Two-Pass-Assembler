assembler: assembler.o addition.o validation.o create.o fileLoops.o printing.o 
		gcc -g -ansi -Wall -pedantic  assembler.o addition.o validation.o create.o fileLoops.o printing.o -o assembler

assembler.o: assembler.c
		gcc -c -g -ansi -Wall -pedantic  assembler.c -o assembler.o
		
addition.o: addition.c
		gcc -c -g -ansi -Wall -pedantic  addition.c -o addition.o
	
validation.o: validation.c
		gcc -c -g -ansi -Wall -pedantic  validation.c -o validation.o
		
create.o: create.c
		gcc -c -g -ansi -Wall -pedantic  create.c -o create.o
		
fileLoops.o: fileLoops.c
		gcc -c -g -ansi -Wall -pedantic  fileLoops.c -o fileLoops.o
						
printing.o: printing.c
		gcc -c -g -ansi -Wall -pedantic  printing.c -o printing.o

