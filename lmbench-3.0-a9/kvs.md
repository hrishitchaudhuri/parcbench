```
gcc -O -DRUSAGE -DHAVE_uint=1 -DHAVE_int64_t=1 -DHAVE_pmap_clnt_h -DHAVE_socklen_t -DHAVE_DRAND48 -DHAVE_SCHED_SETAFFINITY=1   -o ../bin/x86_64-linux-gnu/par_rw par_rw.c ../bin/x86_64-linux-gnu/lmbench.a -lm -lpthread
```