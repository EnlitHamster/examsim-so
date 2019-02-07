# OPTIONS
C-OPTS = -Wall -c
L-OPTS = -Wall -lm
DBG =
LOG =
LDBG =

# PUBLIC CALLS

# build the release
all: log dbgs build
r: build
# build with the logger enabled
l: log
# build with default debugger enabled
d: dbg
# build with severe debugger only enabled
s: dbgs
# build with logger debugger only enabled
g: dbgl

# BUILD OPTIONS
build: manager student clean-obj clean-ipc

# OPTION TARGETS
log:
	$(eval LOG := -D LOG)
dbgl:
	$(eval LDBG := -D LOG_DEBUG)
dbg:
	$(eval DBG := -D DEBUG)
dbgs:
	$(eval DBG := -D DEBUG_SEVERE)

# LINKING TARGETS
manager: debugger.o logger.o manager.o sync.o reader.o reader.h
	gcc manager.o sync.o reader.o logger.o debugger.o -o manager.out $(L-OPTS)
student: student.o sync.o logger.o
	gcc student.o sync.o logger.o -o student.out $(L-OPTS)

# COMPILING TARGETS
logger.o: logger.c bool.h
	gcc $(C-OPTS) $(LDBG) $(LOG) logger.c
debugger.o: debugger.c bool.h
	gcc $(C-OPTS) $(DBG) debugger.c
manager.o: manager.c lib.h
	gcc $(C-OPTS) $(DBG) manager.c
student.o: student.c lib.h
	gcc $(C-OPTS) $(DBG) student.c
reader.o: reader.c bool.h
	gcc $(C-OPTS) $(DBG) reader.c
sync.o: sync.c bool.h
	gcc $(C-OPTS) $(DBG) sync.c

# UTILITY TARGETS
clean-obj:
	rm -f *.o
clean-out:
	rm -f *.out
clean-log:
	rm -rf logs/
clean-ipc:
	ipcrm -a
clean: clean-obj clean-out clean-log clean-ipc
