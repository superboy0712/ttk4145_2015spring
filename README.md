# ttk4145_2015spring
The exercises and project of course ttk4145

## primary layout
each exercise has a folder, with relative text forked from the TA's source

## Useful Visual Reference of git
[ A Visual Git Reference ](https://marklodato.github.io/visual-git-guide/index-en.html#detached)

## How to make a contribution ? ( how to let me merge your commits) 
Spend 5 mins to have a look at this guide [Understanding the github flow](https://guides.github.com/introduction/flow/)

Here's a animated demo
![demo](https://cloud.githubusercontent.com/assets/296432/4485188/881f7a92-49c6-11e4-83dc-bee67d89c139.gif)


##Good Summary of Synchronization Primitives

*[Synchronization primitives](http://www.cs.columbia.edu/~hgs/os/sync.html)*

Note that there are no "official" definitions for these terms, so different texts and implementations associate slightly different characteristics with each primitive.

####semaphores

Two operations, performed by any thread:

original Dijkstra |`P()`|`V()`
-----------------|-----|-----
Tanenbaum |`down()` |`up()`
POSIX |	`sem_wait()` |`sem_post()`
Silberschatz |`wait()` |`signal()`
operation |`while (s==0) {wait}; s--` |`s++`

Note that signals are saved, unlike condition variables. Useful for counting resources (initial value > 1), to implement mutexes (initial value 1) or to signal completion of code across threads (initial value 0). Some semaphore implementations allow decrementing by more than one. 

####mutex
Also known as a lock. Supports lock/unlock operation. In many implementations, the same thread must lock and unlock, but different threads can share the same mutex. POSIX says: "Mutexes have ownership, unlike semaphores. Although any thread, within the scope of a mutex, can get an unlocked mutex and lock access to the same critical section of code, only the thread that locked a mutex can unlock it." Can be implemented via a semaphore. 

####binary semaphore
Semaphore that can take two values, 0 and 1. Can be used as a mutex or to signal completion of code across threads. 
####locks
    Same as mutex. 

####events (from Tanenbaum, Nutt)
    Nutt: similar to condition variables, without the mutex 

####signals (from Tanenbaum)
Signals are "interrupts" that cause a process to asynchronously either abort or jump to a signal handler designated by the process. Pending system calls are interrupted and return an error indication. Also used in conjunction with semaphores and condition variables. 

####condition variables
Condition variables allow several threads to share a monitor (region). Condition variables support two operations, signal() and wait(). Some implementations (Java, POSIX) also support a "broadcast" signal (notifyAll() in Java, `pthread_cond_broadcast` in POSIX). A thread that reaches a wait in a monitor always waits until another thread sends a signal. If several threads are waiting on a condition variable, signal awakens one of them, while the broadcast signal awakens all.
The choice of threads being signaled depends on the implementation. Most implementations make no guarantees.
Condition variables are not counters (i.e., unlike semaphors). Signals do not accumulate. If there is nobody waiting for a signal, the signal is lost. 

####AND synchronization (Nutt, p. 222)
Obtains all required semaphores or none at all. Can possibly be implemented using a T&S with a bit mask. 

####monitors
Class or segment of code that can only be executed by one thread at a time. Variables within the monitor can only be accessed by routines in the monitor. Monitor often use condition variables to allow blocking within the monitor. 

####Test-and-set machine instructions
Atomic instruction that sets the value of a memory location to "true". It returns the value the memory location had before setting it. It can be used to implement semaphores and locks. It does not block. 

*property* |	*lock* |	*semaphore* |	*condition variable*|
-----------|---------|--------------|----------------------
method that blocks calling thread (ANSI)| 	`acquire(), mutex_lock()` |	`sem_wait()` |	`pthread_cond_wait()`
method that releases |	`mutex_unlock() (same thread)`| `sem_post() (same or other thread)`| `pthread_cond_signal() (other thread)`
method that probes without blocking |	`pthread_mutex_trylock()`| 	`sem_trywait()` |	not available
behavior of first thread to reach wait| 	doesn't block |	doesn't block unless semaphore initialized to zero |	blocks
data members |	```List *threads;   /* waiting threads */   boolean locked;```|```List *threads;    unsigned int count;```|	```List *threads;   Lock *mutex;```

##FAQ

###Is a lock variable and a mutex the same thing?
Yes. Unfortunately, you'll also find mutex used as the name for a semaphore variable (e.g., Silberschatz, p. 174) 
###Is an event and a condition variable the same thing, except an event signals all processes waiting on it and a condition variable signals only one process?
Condition variables are used with monitors, i.e., in association with a mutex, events are used by themselves. Both can use "broadcast" signaling, as in notifyAll in Java. 
###Is a mutex and a conditional variable the same thing except a mutex has a value associated with it and a conditional variable does not?
No, if no other process has locked the mutex, the thread will "pass by" the mutex without waiting. For a condition variable, the thread will always wait until being signaled. 
How is thread blocking really implemented?
Text books typically show some kind of waiting operation that blocks the thread when it needs to wait for a resource. However, this doesn't really work unless you had some kind of message system and a single thread per processor. Thus, in reality, a thread will put itself on the queue for a synchronization primitive and then suspend itself. When the synchronization variable unlocks (gets signaled, etc.), the calling process removes the first waiting thread from the queue and puts its back on the ready queue for scheduling. With condition variables, the signaling thread may also suspend itself. 
###What is the typical use of condition variables?
Condition variables not used to protect anything. The associated mutex does, but that's no different from a lock. A CV is a mechanism to temporarily release the lock until some event occurs that makes it sensible for the waiting thread to continue.

Thus, the thread calling the wait() often looks something like
lock to protect variables;
```c
    while (1) {
      wait (until work/message/condition/event arrives, 
        possibly from one of several sources; unlock mutex while waiting);
      get new work, protected by mutex;
    }
```
This is sometimes called a worker thread.

Any of the threads that are sources for work then do
```c
    while (1) {
      read(); /* some blocking operation on a channel/file/network socket */
      get lock;
      put work on some queue protected by lock;
      signal to worker thread;  /* we got work for you! */
      release lock;
    }

```
