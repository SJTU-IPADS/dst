/* Copyright (c) 2009, Fredrik Orderud
   License: BSD licence (http://www.opensource.org/licenses/bsd-license.php) */
#include <stacktrace/stack_exception.hpp>
using namespace stacktrace;

void func_a () {
    throw stack_logic_error("invalid input");
}

class A {
public:
    A () {}
    ~A() {}

    void method () {
        func_a();
    }
};

// make a pure C interface to avoid C++ name mangling issues in DLL interface
extern "C" {

/** Simple function that returns the double of the input for positive values,
    and throws a stack-trace exception on negative input.                    */
__declspec(dllexport) int api_function (const int val) /*throw(stack_logic_error)*/ {
    if (val < 0) {
        // call a method that calls a function that throws, to get some stack-trace entries
        A obj;
        obj.method(); // will throw
    }
    
    // valid input, return twice the input
    return 2 * val;
}

}
