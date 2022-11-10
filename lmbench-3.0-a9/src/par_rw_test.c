#include "bench.h"

// #define XFERSIZE 64 * 1024

typedef struct _state {
    char filename[256];
    int fd;
    int num_readers;
} state_t;

size_t size_;
size_t xfersize;
void* buf;

void do_read(int fd) {
    size_t size, chunk;

    size = size_;
    chunk = xfersize;

    while (size > 0) {
        if (size < chunk)
            chunk = size;

        if (read(fd, buf, size < chunk ? size : chunk) <= 0)
            break;

        bread(buf, size < xfersize ? size : xfersize);
        size -= chunk;
    }
}

void initialize(iter_t iterations, void* cookie) {
    state_t *state = (state_t *) cookie;
    int ofd;

    if (iterations) {
        return;
    }

    if ((int)(ofd = open(state->filename, O_RDONLY)) == -1) { 
        printf("%s\n", state->filename);
        perror("ofd = open(state->filename, O_RDONLY)"); 
        exit(1); 
    }

    state->fd = ofd;
}

void fork_and_read(iter_t iterations, void* cookie) {
    state_t *state = (state_t *) cookie;
    int status;
    pid_t pid[state->num_readers];
    int fd = state->fd;

    while (iterations-- > 0) {
        for (int i = 0; i < state->num_readers; i++) {
            pid[i] = fork();

            if (pid[i] == 0) {
                lseek(fd, 0, SEEK_SET);
                do_read(fd);
                exit(0);
            }

            else if (pid[i] < 0) {
                printf("Failed to fork: %d\n", i);
                status = -1;
            }
        }
    }
}

void cleanup(iter_t iterations, void* cookie) {
    state_t *state = (state_t *) cookie;

    if (iterations) 
        return;

    if (state->fd >= 0)
        close(state->fd);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: ./par_rw <num_readers> <size> <filename>");
        return 0;
    }

    state_t state;

    state.num_readers = atoi(argv[1]);
    size_ = atoi(argv[2]);

    strcpy(state.filename, argv[3]);

    int parallel = 1;
	int warmup = 0;
	int repetitions = -1;

    if (size_ < sizeof(int) * 128) {
        printf("Give more bytes\n");
        exit(1);
    }

    if (size_ < XFERSIZE)
        xfersize = size_;
    else
        xfersize = XFERSIZE;

    buf = (void *) valloc(XFERSIZE);
    bzero(buf, XFERSIZE);

    benchmp(initialize, fork_and_read, cleanup,
			0, parallel, warmup, repetitions, &state);

    bandwidth(size_, get_n() * parallel, 1);
    return 0;
}