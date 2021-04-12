/* Copyright (c) 2009, Fredrik Orderud
   License: BSD licence (http://www.opensource.org/licenses/bsd-license.php) */
#include <iostream>
#include <stdexcept>
#include "auto_dll.hpp"
#include <stacktrace/stack_exception.hpp>
using namespace std;

// function signature for "api_function" in DLL
typedef int(api_function)(const int val);


/** Test code for throwing a stack-enhanced exception across DLL boundaries,
    when using run-time loaded DLLs (should be the trickiest case). */
int main () {
    // load DLL and retrieve function pointer
    auto_dll double_dll("test_lib.dll");
    api_function * fun = double_dll.get_address<api_function>("api_function");

    // catch with std::exception
    try {
        fun(-1); // will thow a stack-enhanced exception
    } catch (const std::exception & e) {
        // print message & stack-trace
        cout << e.what() << endl;
    }

    // catch with stacktrace::stack_exception
    try {
        fun(-1); // will thow a stack-enhanced exception
    } catch (const stacktrace::stack_exception_base & e) {
        // print only stack-trace
        cout << e.to_string() << endl;
    }
}
