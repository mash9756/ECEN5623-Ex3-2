# ECEN5623-Ex3-2

## *2. Review the terminology guide (glossary in the textbook)*
### *A) Describe clearly what it means to write "thread safe" functions that are "re-entrant". There are generally three main ways to do this: 1) pure functions that use only stack and have no global memory, 2) functions which use thread indexed global data, and 3) functions which use shared memory global data but synchronize access to it using a MUTEX semaphore critical section wrapper.*

A thread safe function is a function that can be run in concurrent threads and not cause any data corruption of shared resources. A reentrant function is a function that can run successfully regardless of any preemption or interruption that may occur during execution and can be safely called again even if the previous execution of said function did not complete.

### *B) Describe each method and how you would code it and how it would impact real-time threads/tasks.*

A function that does not manipulate any global memory does not attempt to access shared resources directly, and only operates on data in its defined stack space. They could instead use local copies of a shared resource if necessary that the caller could provide. This could cause complications in data validity based on when the shared resource is copied and when it is used, but it completely avoids access to the shared resource directly.

Thread indexed

Mutex semaphores are used to lock access to critical sections of code to prevent data corruption across threads. The implementation primarily is based around locking the mutex semaphore for as short as possible around as little code as possible to prevent unnecessary blocking or delays in execution of other threads. Mutex semaphores can be used successfully in many applications where access to shared resources is required, but where they are specifically placed should be carefully considered as they can cause deadlocks between threads waiting for the lock and threads releasing the lock if their priorities differ.
