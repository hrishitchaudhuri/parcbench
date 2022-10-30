char *id = "$Id$\n";

#include "bench.h"

#define MAX_LOAD_PARALLELISM 16

void    *buf;
size_t  xfersize;
size_t  count;

typedef struct _state {
    char filename[256];
    int fd;
    int clone;
} state_t;

void doreads(int fd) {
    size_t size, chunk;

    size = count;
    chunk = xfersize;

    while (size > 0) {
        if (size < chunk) chunk = size;

        if (read(fd, buf, (size < chunk ? size : chunk)) <= 0) {
            break;
        }

        bread(buf, size < xfersize ? size : xfersize);
        size -= chunk;
    }
}

// iter_t is an unsigned long
void initialize(iter_t iterations, void* cookie) {
    state_t *state = (state_t *) cookie;
}