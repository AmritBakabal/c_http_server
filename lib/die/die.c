#include "../../include/die/die.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

int log_die(const char *filename, int line, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "\033[1;31mCASE:\033[0m[%s:%d] %s: ", filename, line, strerror(errno));
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(EXIT_FAILURE);
}

int log_error(const char *filename, int line, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "\033[1;31mERROR:\033[0m[%s:%d] %s: ", filename, line, strerror(errno));
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  va_end(args);
}

int log_warn(const char *filename, int line, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "\033[1;33mWARN:\033[0m[%s:%d] %s: ", filename, line, strerror(errno));
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  va_end(args);
}

int log_info(const char *filename, int line, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "\033[1;32mINFO:\033[0m[%s:%d] %s: ", filename, line, strerror(errno));
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  va_end(args);
}