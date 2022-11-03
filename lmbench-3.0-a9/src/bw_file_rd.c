/*
 * bw_file_rd.c - time reading & summing of a file
 *
 * Usage: bw_file_rd [-C] [-P <parallelism] [-W <warmup>] [-N <repetitions>] size file
 *
 * The intent is that the file is in memory.
 * Disk benchmarking is done with lmdd.
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
char *id = "$Id$\n";

#include "bench.h"

#define CHK(x)          \
	if ((int)(x) == -1) \
	{                   \
		perror(#x);     \
		exit(1);        \
	} // perror prints a textual description of the error code stored in system variable errno to stderr
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b)) // Returns min of a and b
#endif

#define TYPE int
#define MINSZ (sizeof(TYPE) * 128)

void *buf;		 /* do the I/O here */
size_t xfersize; /* do it in units of this */
size_t count;	 /* bytes to move (can't be modified) */

// size_t is an unsigned data type. Generally used to count a quantity that cannot be negative. Can be any unsigned data type depending on the implementation.

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
	if (state->clone)
	{
		char buf[128];
		char *s;

		/* copy original file into a process-specific one */
		sprintf(buf, "%d", (int)getpid());
		s = (char *)malloc(strlen(state->filename) + strlen(buf) + 1);
		sprintf(s, "%s%d", state->filename, (int)getpid());
		if (cp(state->filename, s, S_IREAD | S_IWRITE) < 0)
		{
			perror("creating private tempfile");
			unlink(s);
			exit(1);
		}
		strcpy(state->filename, s);
	}
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

void time_with_open(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *)cookie;
	char *filename = state->filename;
	int fd;

	while (iterations-- > 0)
	{
		fd = open(filename, O_RDONLY);
		doit(fd);
		close(fd);
	}
}

void time_io_only(iter_t iterations, void *cookie)
{
	state_t *state = (state_t *)cookie;
	int fd = state->fd;

	while (iterations-- > 0)
	{
		lseek(fd, 0, SEEK_SET); // Moves file pointer to beginning of file descriptor
		doit(fd);
	}
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
		benchmp(initialize, time_with_open, cleanup,
				0, parallel, warmup, repetitions, &state);
	}
	else if (!strcmp("io_only", av[optind + 1]))
	{
		benchmp(init_open, time_io_only, cleanup,
				0, parallel, warmup, repetitions, &state);
	}
	else
		lmbench_usage(ac, av, usage);
	bandwidth(count, get_n() * parallel, 1);
	// fprintf(stderr, "Hi!\n");
	return (0);
}
