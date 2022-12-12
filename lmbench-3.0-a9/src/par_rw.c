#include "bench.h"
#include <pthread.h>
#include <time.h>
#include "lib_timing.c"
#include <sys/syscall.h>

#define NUM_THREADS 4

#define gettidv1() syscall(__NR_gettid) // new form
#define MINSZ (sizeof(TYPE) * 128)
#define CHK(x)          \
	if ((int)(x) == -1) \
	{                   \
		perror(#x);     \
		exit(1);        \
	}

void *buf;		 /* do the I/O here */
size_t xfersize; /* do it in units of this */
size_t count;	 /* bytes to move (can't be modified) */

struct arg_struct {
	int iterations;
	void *cookie;
	int p_id;
};

typedef struct _state
{
	char filename[256];
	int fd;
	int clone; // if set to true, process first copies file into local buffer/process specific file and reads from that instead
} state_t;

void doit(int fd)
{
	size_t size, chunk;

	size = count;
	chunk = xfersize;
	while (size > 0)
	{
		// While there is something to be read
		if (size < chunk)
			chunk = size;
		// If size is less than chunk, then chunk is set to size, done when remaining amount to read is less than chunk
		if (read(fd, buf, MIN(size, chunk)) <= 0)
		{
			// Reads chunk bytes from fd into buf
			// If negative return value, it is an error. If it is 0, it is EOF. Generally, return value is number of bytes read.
			break;
		}
		bread(buf, MIN(size, xfersize));
		// bread is doing some sort of block read, not entirely sure
		size -= chunk;
	}
}

void initialize(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *)cookie;

	if (iterations)
		return;

	state->fd = -1;
	// if (state->clone)
	// {
	// 	char buf[128];
	// 	char *s;

	// 	/* copy original file into a process-specific one */
	// 	sprintf(buf, "%d", (int)getpid());
	// 	s = (char *)malloc(strlen(state->filename) + strlen(buf) + 1);
	// 	sprintf(s, "%s%d", state->filename, (int)getpid());
	// 	if (cp(state->filename, s, S_IREAD | S_IWRITE) < 0)
	// 	{
	// 		perror("creating private tempfile");
	// 		unlink(s);
	// 		exit(1);
	// 	}
	// 	strcpy(state->filename, s);
	// }
}

void init_open(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *)cookie;
	int ofd;

	if (iterations)
		return;

	initialize(0, cookie);
	CHK(ofd = open(state->filename, O_RDONLY));
	state->fd = ofd;
}

void *time_with_open(void *arguments)
{
	struct arg_struct *args = arguments;
	state_t *state = (state_t *)args->cookie;
	char *filename = state->filename;
	int itr = args->iterations; 
	int fd;
	//printf("\n I am doing time with open, p_id: %ld", (long int)gettidv1());

	while (itr-- > 0)
	{
		if ((int)(fd = open(state->filename, O_RDONLY)) == -1) { 
			printf("%s\n", state->filename);
			perror("fd = open(state->filename, O_RDONLY)"); 
			exit(1); 
    	}

    	//printf("\n Iteration, [INIT] File descriptor: %d %d\n", itr, fd);
		state->fd = fd;

		doit(state->fd);
		close(state->fd);
	}
	//printf("I am done with time with open, p_id: %ld", (long int)gettidv1());
	// pthread_exit(NULL);
	return NULL;
}

void *time_io_only(void *arguments)
{
	struct arg_struct *args = arguments;

	state_t *state = (state_t *)args->cookie;
	char *filename = state->filename;
	int itr = args->iterations; 
	int fd = state->fd;
	//printf("\n I am doing time io, p_id: %ld", (long int)gettidv1());

	while (itr-- > 0)
	{
		lseek(state->fd, 0, SEEK_SET); // Moves file pointer to beginning of file descriptor
		doit(state->fd);

	}
	//printf("I am done with time io, p_id: %ld", (long int)gettidv1());
	// pthread_exit(NULL);
	return NULL;

}

void cleanup(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *)cookie;

	if (iterations)
		return;

	if (state->fd >= 0)
		close(state->fd);
	if (state->clone)
		unlink(state->filename);
}

void parallel_open(iter_t iterations, void *cookie){
    //printf("----------------------------------------------------");
    pthread_t thread[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i ++)
    {
		struct arg_struct args;
		args.p_id = i;
		args.iterations = iterations;
		args.cookie = cookie;
        pthread_create(&thread[i], NULL, time_with_open, (void *)&args);
    	//printf("\nPOpen - Created thread %ld",(long int)gettidv1());
    }
    for (int i = 0; i < NUM_THREADS; i ++)
    {
    	//printf("\nPOpen - Joining thread %ld",(long int)gettidv1());
        pthread_join(thread[i], NULL);
    }
    //printf("----------------------------------------------------");

}

void parallel_seek(iter_t iterations, void *cookie) {
    //printf("yo bro parallel seek");
	pthread_t thread[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i ++)
    {
		struct arg_struct args;
		args.iterations = iterations;
		args.cookie = cookie;
		args.p_id = i;
        pthread_create(&thread[i], NULL, time_io_only, (void *)&args);
		//printf("\nPSeek - Created thread %ld",(long int)gettidv1());
    }
    for (int i = 0; i < NUM_THREADS; i ++)
    {
    	//printf("\nPSeek - Joining thread %ld",(long int)gettidv1());
        pthread_join(thread[i], NULL);
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

	//fprintf(stderr, "Begin\n");
	fprintf("#Threads: %d\n",NUM_THREADS);
	if (!strcmp("open2close", av[optind + 1]))
	{
		benchmp(initialize, parallel_open, cleanup,
				0, parallel, warmup, repetitions, &state);
		//printf("hello mr i'm done");
	}
	else if (!strcmp("io_only", av[optind + 1]))
	{
		benchmp(init_open, parallel_seek, cleanup,
				0, parallel, warmup, repetitions, &state);
	}
	else
		lmbench_usage(ac, av, usage);
	printf("%d,%d,%d",count,get_n(),parallel);
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
