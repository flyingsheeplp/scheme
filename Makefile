CC=cc
CFLAGS=-c -I./include
LD=gcc

target=interpreter

target: interpreter.o dump.o env.o constant.o
	$(LD) interpreter.o dump.o env.o constant.o -o $(target)

interpreter.o: interpreter.c include/interpreter.h include/dump.h include/env.h include/constant.h
	$(CC) $(CFLAGS) $< 

dump.o: dump.c include/interpreter.h include/dump.h include/env.h include/constant.h
	$(CC) $(CFLAGS) $<

env.o: env.c include/interpreter.h include/dump.h include/env.h include/constant.h
	$(CC) $(CFLAGS) $<

constant.o: constant.c include/interpreter.h include/dump.h include/env.h include/constant.h
	$(CC) $(CFLAGS) $<

clean:
	rm *.o
	rm -rf $(target)

.PHONY: clean
