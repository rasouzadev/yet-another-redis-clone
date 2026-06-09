#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

#define K_MAX_MSG 4096

void die(const char *msg);
void msg(const char *msg);
int32_t read_full(int fd, char *buf, size_t n);
int32_t write_all(int fd, const char *buf, size_t n);

#endif
