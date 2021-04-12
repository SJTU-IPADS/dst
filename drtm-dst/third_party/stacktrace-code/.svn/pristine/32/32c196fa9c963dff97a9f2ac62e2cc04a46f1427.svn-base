#include <stdlib.h>
#include "StackTrace.h"

void foo(int n); // calls bar one time, passing n
void bar(int n); // recursively calls self, decrementing n until 0

// calls foo() with first command line argument
// (or '3' if no arguments given)
int main(int argc, char** argv) {
	if(argc>1)
		foo(atoi(argv[1]));
	else
		foo(3);
	return 0;
}

void foo(int n) {
	bar(n);
}

void bar(int n) {
	if(n>0)
		bar(n-1);
	else {
		// Two optional parameters are max depth and initial skip:
		// -1U (default) means unlimited depth (display whole stack)
		// 0 (default) means start with the calling function
		stacktrace::displayCurrentStackTrace(/*-1U, 0*/);
		
		//See StackTrace.h for more options and documentation
	}
}
