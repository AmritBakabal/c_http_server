#include "../../include/die/die.h"
#include <stdlib.h>
#include <stdio.h>

int die_func(const char *filename, int line, const char *msg) {
  fprintf(stderr, "[%s:%d] ", filename, line);
  perror(msg);
  exit(EXIT_FAILURE);
}