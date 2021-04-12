.PHONY: all clean

all: c_sample cpp_sample

clean:
	rm -rf *.o c_sample cpp_sample


# C example:

c_sample: c_sample.o StackTrace_c.o
	$(CC) $^ -o "$@"

c_sample.o: c_sample.c StackTrace.h
	$(CC) -g -c $<

# A little trick... force StackTrace to (re)compile as 'C',
# regardless of file extension
StackTrace_c.o: StackTrace.h StackTrace.cc
	$(CC) -x c -g -c StackTrace.cc -o "$@"


# C++ example:

cpp_sample: cpp_sample.o StackTrace_cpp.o
	$(CXX) $^ -o "$@"

cpp_sample.o: cpp_sample.cc StackTrace.h
	$(CXX) -g -c $<

# A little trick... force StackTrace to (re)compile as 'C++',
# regardless of file extension
StackTrace_cpp.o: StackTrace.h StackTrace.cc
	$(CXX) -x c++ -g -c StackTrace.cc -o "$@"

