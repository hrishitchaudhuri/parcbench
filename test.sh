#!/bin/bash

for((i=0; i<1; i++))
do
		cd lmbench-3.0-a9

		# compile
		cd src
		make clobber
		make lmbench
		# modify executable name
		gcc -O -DRUSAGE -DHAVE_uint=1 -DHAVE_int64_t=1 -DHAVE_pmap_clnt_h -DHAVE_socklen_t -DHAVE_DRAND48 -DHAVE_SCHED_SETAFFINITY=1   -o ../bin/x86_64-linux-gnu/par_rw_test par_rw_test.c ../bin/x86_64-linux-gnu/lmbench.a -lm -lpthread

		# run
		cd ..
		cd bin/x86_64-linux-gnu
		# modify arguments
		./par_rw_test 4 63562 ../../../freqs.json
		echo "DONE"
		exit
done
