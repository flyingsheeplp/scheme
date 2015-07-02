CC=cc
CFLAGS=-c
LD=gcc

target=interpreter

target: interpreter.o
	$(LD) $< -o $(target)

interpreter.o: interpreter.c interpreter.h
	$(CC) $(CFLAGS) $< 

clean:
	rm *.o
	rm -rf $(target)

.PHONY: clean
