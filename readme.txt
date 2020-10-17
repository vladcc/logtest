Minimal out of thread logging poc. Place your message in a queue and leave io
to another thread.

Uses spin locks. With -D TRASH_CACHE you use the inferior one (more or less the
same like in boost). With -D READ_EXCH you use the faster one, but load before
you exchange. Supposedly has better cache performance. Should measure, though.

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
