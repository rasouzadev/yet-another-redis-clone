#include "utils.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int32_t query(int fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > K_MAX_MSG) {
    return -1;
  }

  // Send request
  char wbuf[4 + K_MAX_MSG];
  memcpy(wbuf, &len, 4);
  memcpy(&wbuf[4], text, len);
  int32_t err = write_all(fd, wbuf, 4 + len);
  if (err) {
    return err;
  }

  // Get response header
  char rbuf[4 + K_MAX_MSG];
  errno = 0;
  err = read_full(fd, rbuf, 4);
  if (err) {
    msg(errno == 0 ? "EOF" : "read() error");
  }
  memcpy(&len, rbuf, 4);
  if (len > K_MAX_MSG) {
    msg("too long");
    return -1;
  }

  // Get response body
  err = read_full(fd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }

  printf("server says: %.*s\n", len, &rbuf[4]);
  return 0;
}

int main() {
  // Socket configuration
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // localhost
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("connect");
  }
  
  // Requests
  int32_t err = query(fd, "hello1");
  if (err) {
    goto L_DONE;
  }
  err = query(fd, "hello2");
  if (err) {
    goto L_DONE;
  }
  err = query(fd, "hello3");
  if (err) {
    goto L_DONE;
  }

L_DONE:
  close(fd);
  return 0;
}
