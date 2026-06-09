#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

/*
 * TODO: [Future Improvement - Blocking I/O]
 * For my future self: This function currently relies on synchronous, blocking reads. 
 * If a client sends a partial message and then stalls, the server thread will hang here indefinitely waiting for the remaining bytes.
 * To make this server robust, we need to migrate to:
 * 1. Non-blocking sockets (using fcntl with O_NONBLOCK)
 * 2. An event loop (e.g., epoll/poll) to handle partial reads asynchronously by catching EAGAIN/EWOULDBLOCK errors.
 */
int32_t read_full(int fd, char *buf, size_t n) {
    // Loop until read all bytes
    while (n > 0) {
        // Read the message bytes from the socket and write to buf. It can return chunks, being the reason to loop while n has not reached its end.
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        // Assert that the current bytes are not greater than the remaining size
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

/*
 * TODO: [Future Improvement - Slow Reader Hazard]
 * Just like read_full, this blocking write can hang the server. 
 * If the client stops reading, the OS send buffer will fill up (TCP Flow Control),
 * causing write() to block indefinitely.
 * Future fix: Non-blocking sockets and let the event loop (epoll) signal when the socket is writable again.
 */
int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}
