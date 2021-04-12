# DST, a scalable and general purpose timestamp mechanism for MVCC

Multi-version concurrency control (MVCC) enables more concurrency control than traditional one-version concurrency
control, such that reads operate concurrently to writes by reading from a consistent snapshot.

In MVCC, an import aspect is how to determine the version of tuple to read, and how to determine the version to write.
On a single macine (or centralized database), this is pretty simple: one can use the global counter.
However, in a 

## DST and its application to various database systems

DST is a timestamp mechanism to provide serializable snapshot read.
It's basic idea is to piggyback timestamp selection based on transaction's original concurrency control,
so that the timestamp selection can be freely tracked without accessing any centralized data structures.

DST is a general timestamp scheme that can be applied to various concurrency control protocols,
including [two-phase locking(2PL)](https://www.google.com/search?client=safari&rls=en&q=two+phase+locking&ie=UTF-8&oe=UTF-8) and [optimistic concurrency control(OCC)](https://en.wikipedia.org/wiki/Optimistic_concurrency_control), the most
standard protocols for transaction.
Below is a summary of which system (and their corresponding concurrency control protocols) DST applies to.
Notice that the **lines of code changed (LOC)** only includes changes
to concurrency control layers / data store, which does not include execution frameworks,
or benchmark code.

| Systems       | Codebase      | LOC changed  | Concurrency control protocol | 
| ------------- |:-------------:| ------------------:| :----------------------------:|
| DrTM          | [DrTM](https://ipads.se.sjtu.edu.cn/pub/projects/drtm) | 154                 | Optimistic concurrency control |
| MySQL cluster | [MySQL](https://www.mysql.com/cn/products/cluster/)      | 222                 | Two-phase locking            |
| ROCOCO        | [ROCOCO](https://github.com/shuaimu/rococo)      | 171                 | [ROCOCO](https://www.usenix.org/system/files/conference/osdi14/osdi14-paper-mu.pdf)                      |

## Detailed results

The following is a detailed evaluation results of DST on different systems.
- [DrTM.](./docs/drtm.md)

- [MySQL cluster.](./docs/mysql.md)

- [ROCOCO.](./docs/rococo.md)
  