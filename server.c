#include "utils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

static int32_t one_request(int connfd) {
  // Max msg size + 4 byte header
  char rbuf[4 + K_MAX_MSG];
  // Errno is not set to 0 if syscall succeed by default
  errno = 0;
  int32_t err = read_full(connfd, rbuf, 4);
  if (err) {
    msg(errno == 0 ? "EOF" : "read() error");
    return err;
  }
  uint32_t len = 0;
  memcpy(&len, rbuf, 4);
  if (len > K_MAX_MSG) {
    msg("too long");
    return -1;
  }
  err = read_full(connfd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }
  fprintf(stderr, "client says: %.*s\n", len, &rbuf[4]);
  const char reply[] = "world";
  char wbuf[4 + sizeof(reply)];
  len = (uint32_t)strlen(reply);

  memcpy(wbuf, &len, 4);
  memcpy(&wbuf[4], reply, len);
  return write_all(connfd, wbuf, 4 + len);
}

int main() {
  // AF_INET for IPv4, AF_INET6 for IPv6
  // SOCK_STREAM for TCP, SOCK_DGRAM for 
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  // 2nd and 3rd arguments specifies options to set
  // 4th argument is the option value
  // Options use different types, to the size of the option value is needed
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // sockaddr_in holds an IPv4:port pair stored as big-endiar numbers. For IPv6, use sockaddr_in6 instead.
  // there are 2 ways to store integers in memory:
  // Little-endian: Stores the numbers starting from the least significative byte: 1234 is represented by 4321
  // Big-endian (or network byte order): Stores the numbers starting from the most significative byte: 1234 is 1234
  // Since most relevant new CPUs are little-endian, we need to perform a byte swap operation.
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234); // Host to Network Short 16bit (or CPU Endian (Little-endian) to Network Endian (Big-endian)). Set port
  addr.sin_addr.s_addr = htonl(0); // Same as above, but 32bit. Set a wildcard IP 0.0.0.0 

  // struct sockaddr_in and struct sockaddr_in6 have different sizes, that is why addrlen is needed.
  // the struct sockaddr is not used anywhere, so we cast the sockaddr_in or sockaddr_in6 into it to match bind expected parameters
  int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) { die("bind()"); }

  // This is where the socket is actually created. The OS will handle TCP handshakes and place established connections in a queue, then we can retrieve them via "accept()".
  rv = listen(fd, SOMAXCONN); // 4096 linux 
  if (rv) { die("listen()"); }
  
  while (1) {
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    if (connfd < 0){
      continue;
    }

    while (1) {
      int32_t err = one_request(connfd);
      if (err) {
        break;
      }
    }

    close(connfd);
  }

  return 0;
}
