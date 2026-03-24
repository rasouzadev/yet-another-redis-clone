#include <stdio.h>
#include <stdlib.h>

void die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void msg(const char *msg) {
  fprintf(stderr, "%s\n", msg);
}
