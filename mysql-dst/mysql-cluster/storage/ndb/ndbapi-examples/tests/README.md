Code layout:
## utilies:
- dbheader.hpp:       common constants, and macros
- timer.hpp:          utilies for counting times
- random.hpp:         fast random

## main tests
- tx_workload.hpp:    definiation of a TX workload
- test_main.cc        main start hook of the execution framework
- test_funcs          entries of different test funcs
- test_client.hpp/cc  main test event loop


## some minor test files
- micro_read_client.hpp  some simple tests using NDBApi
- micro_main.cc          bootstrap file (main start hook) of micro_read_client.hpp


Add a benchmark:
    A benchmark has several test functions.
    First, define the workload the benchmark should run. For example, if my workload has 50% of { read A},50% of {WRITE A}, I will define the following according to tx_workload.hpp:
    ```c++
      txn_result_t read(Ndb &n,FastRandom &r) {
           // read A
      }
 
      txn_result_t write(Ndb &n,FastRandom &r) {
           // write A
      }
    ```
    Then, define the workload by telling how often each of the above functions should run.
      ```c++
      workload_desc_vec_t generate_test_funcs() {
          auto ret = workload_desc_vec_t();
          ret.emplace_back("update",0.5,write);
          ret.emplace_back("read",0.5,read);
          return ret;
      }
      ```

   Finally, bind the above workload to a specific benchmark, modifies the `init_workload` in test_funcs.cc by adding the above descriptions of the workload:
     ```c++
       supported_workloads.insert(std::make_pair("AWORKLOAD",generate_test_funcs));
     ```
  Now its all set. Now you can use `./tester 0 "/home/wxd/mysql-base/mysql.sock" "val00" "AWORKLOAD"` to evaluate the workload! 