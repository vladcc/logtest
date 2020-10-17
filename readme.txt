Minimal out of thread logging poc.
Uses spin locks. With -D=TRASH_CACHE you use the inferior one (more or less the
same like in boost). ~2x slower at 10 million cycles.
