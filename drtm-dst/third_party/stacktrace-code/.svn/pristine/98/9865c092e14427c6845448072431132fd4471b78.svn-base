# Use GNU C++ compiler
CC=g++
# Compiler settings (warning on all)
CFLAGS=-c -Wall -I.

all: obj/call_stack_gcc.o obj/test_code.o
	$(CC) obj/call_stack_gcc.o obj/test_code.o -ldl -rdynamic -o test_code

obj/call_stack_gcc.o:
	$(CC) $(CFLAGS) stacktrace/call_stack_gcc.cpp -o obj/call_stack_gcc.o

obj/test_code.o:
	$(CC) $(CFLAGS) test/test_code.cpp -o obj/test_code.o

clean:
	rm -rf obj/*.o test_code
