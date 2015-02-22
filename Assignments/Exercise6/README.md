Exercise 6 : Phoenix
====================
This exercise must be approved at the RT-lab by a student assistent by the deadline on itsLearning.

####1: Process pairs
Create a program (in any language, on any OS) that uses the _process pair_ technique to print the numbers `1`, `2`, `3`, `4`, etc to a terminal window. The program should create its own backup: When the primary is running, _only the primary_ should keep counting, and the backup should do nothing. When the primary dies, the backup should become the new primary, create its own new backup, and keep counting where the dead one left off. Make sure that no numbers are skipped!

You cannot rely on the primary telling the backup when it has died (because it would have to be dead first...). Instead, have the primary broadcast that it is alive a few times a second, and have the backup become the primary when a certain number of messages have been missed.

You will need some form of communication between the primary and the backup. Some examples are:
 - Network: The simplest is to use UDP on localhost. TCP is also possible, but may be harder (since both endpoints need to be alive).
 - IPC, such as POSIX message queues: see [`msgget()` `msgsnd()` and `msgrcv()`](http://pubs.opengroup.org/onlinepubs/7990989775/xsh/sysmsg.h.html). With these you can create FIFO message queues.
 - [Signals](http://pubs.opengroup.org/onlinepubs/7990989775/xsh/signal.h.html): Use signals to interrupt other processes (You are already familiar with some of these, such as SIGSEGV (Segfault) and SIGTERM (Ctrl+C)). There are two custom signals you can use: SIGUSR1 and SIGUSR2. See `signal()`.
   - Note for D programmers: [SIGUSR is used by the GC.](http://dlang.org/phobos/core_memory.html)
 - Files: The primary writes to a file, and the backup reads it. Either the time-stamp of the file or the contents can be used to detect if the primary is still alive.
 - Controlled shared memory: The system functions [`shmget()` and `shmat()`](http://pubs.opengroup.org/onlinepubs/7990989775/xsh/sysshm.h.html) let processes share memory.

You will also need to spawn the backup somehow. There should be a way to spawn processes or run shell commands in the standard library of your language of choice. The name of the terminal window is OS-dependent:
 - Ubuntu: `gnome-terminal`, with option `-e` to run a program in a new window
 - Mint: `mate-terminal`, with option `-x` to run a program in a new window
 - Windows: `start "title" program_name`. Note that you _must_ specify a title.
 
(Linux tip: You can prevent a spawned terminal window from automatically closing by going to Edit -> Profile Preferences -> Title and Command -> When command exits. Windows tip: Use `start "title" call program_name`)

Be careful! You don't want to create a [chain reaction](http://en.wikipedia.org/wiki/Fork_bomb)... If you do, you can use `pkill -f program_name` (Windows: `taskkill /F /IM program_name /T`) as a sledgehammer.

(Note: because of a delay with logging on terminals on the RT-lab computers you won't be able to start new terminals from within your program. You can and should do this on your personal computer, but we will accept manual starts of each process.) 
(Note for the project: Usually a program crashes for a reason. Restoring the program to the same state as it died in may cause it to crash in exactly the same way, all over again.)

 
####2: Guarantees (Optional)
Make your program print each number once and _only_ once, and demonstrate (a priori, not just through observation of your program) that it will behave this way, regardless of when the primary is killed.

##Partial objective
Each of the following excercises will include a partial objective for the project. If you follow these you will experience less stress in week 16-17 than if you postphone everything to the last few weeks. 

This weeks goal is getting one elevator to run. Create a clearly defined interface to the elevator control-module so that the rest of the system can easely control the elevator. Use pen and paper, discuss among yourself how this interface should work before you implement your solution. KISS - Keep It Simple, Stupid. 

If you have created your own objectives present this, and your work, to the student assistent.
