#include "bench.h"
#include <pthread.h>
#include <time.h>
#include "lib_timing.c"
#include "bw_file_rd.c"

typedef struct _state
{
	char filename[256];
	int fd;
	int clone; // if set to true, process first copies file into local buffer/process specific file and reads from that instead
} state_t;

void parallel_open(iter_t iterations, void *cookie){
    for (int i = 0; i < 2; i ++)
    {
        pthread_t thread;
        pthread_create(&thread, NULL, time_with_open, NULL);
    }
}

void parallel_seek(iter_t iterations, void *cookie) {
    for (int i = 0; i < 2; i ++)
    {
        pthread_t thread;
        pthread_create(&thread, NULL, time_io_only, NULL);
    }
}

int main(int ac, char **av)
{
	int fd;
	state_t state;
	int parallel = 1;
	int warmup = 0;
	int repetitions = -1;
	int c;
	char usage[1024];
	// -w is the amount of the time the execution has to be delayed by
	sprintf(usage, "[-C] [-P <parallelism>] [-W <warmup>] [-N <repetitions>] <size> open2close|io_only <filename>"
				   "\nmin size=%d\n",
			(int)(XFERSIZE >> 10));
	// XFERSIZE = 64 KB
	state.clone = 0;
	// getopt is used to parse command line args
	// it stores value chosen is variable `optarg`
	while ((c = getopt(ac, av, "P:W:N:C")) != EOF)
	{
		switch (c)
		{
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0)
				lmbench_usage(ac, av, usage); // prints how to use lmbench
			break;
		case 'W':
			warmup = atoi(optarg);
			break;
		case 'N':
			repetitions = atoi(optarg);
			break;
		case 'C':
			state.clone = 1;
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}
	// optind is the index of next element to be processed in argument counter
	if (optind + 3 != ac)
	{ /* should have three arguments left */
		// required args are size, open2close/io_only, filename
		lmbench_usage(ac, av, usage);
	}

	strcpy(state.filename, av[optind + 2]); // copy filename to state.filename
	count = bytes(av[optind]);				// converts size to bytes?
	if (count < MINSZ)
	{
		// if the size is lesser than the minimum size, exit
		// fprintf(stderr, "Took this exit\n");
		exit(1); /* I want this to be quiet */
	}
	if (count < XFERSIZE)
	{
		xfersize = count;
	}
	else
	{
		xfersize = XFERSIZE;
	}
	buf = (void *)valloc(XFERSIZE); // Reserves memory which is page aligned, i.e., allocates virtual memory?
	bzero(buf, XFERSIZE);			// erases XFERSIZE of memory starting at buf

	// fprintf(stderr, "Begin\n");
	if (!strcmp("open2close", av[optind + 1]))
	{
		benchmp(initialize, parallel_open, cleanup,
				0, parallel, warmup, repetitions, &state);
	}
	else if (!strcmp("io_only", av[optind + 1]))
	{
		benchmp(init_open, parallel_seek, cleanup,
				0, parallel, warmup, repetitions, &state);
	}
	else
		lmbench_usage(ac, av, usage);
	bandwidth(count, get_n() * parallel, 1);
	// fprintf(stderr, "Hi!\n");
	return (0);
}
    /*
        lib_timing.c : contains benchmp(benchmp_f initialize, 
	                                    benchmp_f benchmark,
	                                    benchmp_f cleanup,
	                                    int enough, 
	                                    int parallel,
	                                    int warmup,
	                                    int repetitions,
	                                    void* cookie)

        benchmp_f : typedef void (*benchmp_f)(iter_t iterations, void* cookie);
            (function pointer to void fn that takes an iter_t (long) and cookie)

        initialize, benchmark, cleanup : functions obeying the benchmp_f signature
            initialize -> executed initially
            benchmark -> code that is actually benchmarked
            cleanup -> necessary destructor procedures


        enough : a param that decides how long to run the bench for.
            if 0 -> lmbench decides how long to run it
            if !0 -> timeout, unless lmbench thinks enough is too short


        parallel : like the name suggests; multiple processes execute the benchmark
                    concurrently. 

        
        warmup : how long to delay each child's run for.


        NOTE : benchmp actually executes at least one child process to run the benchmarks
            (the function is literally just a signal collector.)
    */


    /*
        Big Idea 1: 
                Benchmark function spawns multiple reader-writer processes. 
                All try to read/write to the same file. 
    */
