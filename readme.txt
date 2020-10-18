Note: spin locks make sense only when you know you're being scheduled; schedule
real time, isolate a core, be the kernel, etc. I.e. makes sense in a
non-preemptible context. Either that or *very* little spinning.
https://www.realworldtech.com/forum/?threadid=189711&curpostid=189723

Minimal out of thread logging poc. Place your message in a queue and leave io
to another thread.

Uses spin locks. With -D TRASH_CACHE you use the inferior one (more or less the
same like in boost). With -D READ_EXCH you use the faster one, but load before
you exchange. Seems to be faster. Should measure, though. -D REPORT_QSIZE
reports the queue size to stdout on each logger loop iteration. If you really
want to, you can compile with -D MUTEX, and then the spinlock becomes a
std::mutex. c++17 required.

Use: ./prog <how-many-loops> <core-main-thread> <core-logger-thread>

If <core-main-thread> and <core-logger-thread> are not numbers, then you'd see
warnings and the threads will receive default scheduling.

Compile:
cat compile.txt | bash

Clean:
cat clean.txt | bash

Logging goes to stdout, so run like:
./prog <how-many-loops> <core-main-thread> <core-logger-thread> > /dev/null
to see only the measurement results.
