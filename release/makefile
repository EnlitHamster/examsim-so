# OPTIONS
C-OPTS = -Wall -c
L-OPTS = -Wall -lm

all: manager student clean-obj

manager: manager.o sync.o reader.o
	gcc manager.o sync.o reader.o -o manager.out $(L-OPTS)
student: student.o sync.o
	gcc student.o sync.o -o student.out $(L-OPTS)

manager.o: manager.c lib.h
	gcc $(C-OPTS) manager.c
student.o: student.c lib.h
	gcc $(C-OPTS) student.c
sync.o: sync.c bool.h
	gcc $(C-OPTS) sync.c
reader.o: reader.c bool.h
	gcc $(C-OPTS) reader.c

clean-obj:
	rm -f *.o
clean-out:
	rm -f *.out
clean: clean-obj clean-out
