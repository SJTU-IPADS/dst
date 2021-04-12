/* Copyright (c) 2009, Fredrik Orderud
   License: BSD licence (http://www.opensource.org/licenses/bsd-license.php) */

#include <iostream>
#include <stacktrace/call_stack.hpp>
#include <stacktrace/stack_exception.hpp>

using namespace std;
using namespace stacktrace;

void func_b () {
    call_stack st;
    cout << st.to_string();
}
void func_a () {
    func_b();
}

class C {
public:
    C () {}

    void method () {
        call_stack st;
        cout << st.to_string();
    }
};

void func_bug () {
    throw stack_runtime_error("some error occured");
}

/** stacktrace test code. */
int main (int /*argc*/, const char* /*argv*/[]) {
    cout << "Stack trace from a nested function:" << endl;
    func_a();

    cout << "Stack trace from a method:" << endl;
    C obj;
    obj.method();

    cout << "Stack trace from exception:" << endl;
    try {
        func_bug(); // will throw
    } catch (const exception & e) {
        cout << e.what();
    }

    return 0;
}
