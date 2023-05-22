#include <stdarg.h>
#include <string.h>
#include "../../include/stringbuffer/stringbuffer.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/**
 * @brief doubles the string_buffer size
 * 
 * @param buffer 
 * @return char * 
 */
char *string_buffer_expand(struct string_buffer *buffer) {
  buffer->str = realloc(buffer->str, 2 * buffer->capacity * sizeof(char));
  if(buffer->str != NULL)  {
    // realloc success; update capacity
    buffer->capacity *= 2;
  }
  return buffer->str;
}

/**
 * @brief Writes the given formatted string to the string_buffer
 * 
 * @param buffer 
 * @param fmt format string 
 * @param ... format specifiers' arguments
 * @return int total number of characters written to the buffer
 */
int string_buffer_write(struct string_buffer *buffer, const char *fmt, ...) {
  va_list args;
  int written_count;
  va_start(args, fmt);
  // try to write to buffer by expanding it, if needed.
  while (1) {
    written_count = vsnprintf(buffer->str + buffer->size,
                              buffer->capacity - buffer->size, fmt, args);
    if (written_count >= (buffer->capacity - buffer->size)) {
      if(string_buffer_expand(buffer) == NULL) {
        written_count = -1;
        break;
      }
    } else{
      // sucessfully written `fmt` to buffer
      // so, update the buffer size
      buffer->size += written_count;
      break;
    }
  }
  va_end(args);
  if(written_count < 0) {
    buffer->str[buffer->size] = '\0';
    assert(written_count > -1);
  }
  return written_count;
}

/**
 * @brief Initializes the given `struct string_buffer *buffer`
 * The string is empty "" by default
 * 
 * @param buffer 
 * @returns pointer to sthe struct
 */
struct string_buffer *string_buffer_create() {
  struct string_buffer *buffer = (struct string_buffer *) malloc(sizeof(struct string_buffer));
  assert(buffer != NULL && "error while allocating buffer");
  buffer->capacity = 65536;
  buffer->size = 0;
  buffer->str = (char *)malloc(buffer->capacity * sizeof(char));
  assert(buffer->str != NULL && "error while allocating buffer's str");
  buffer->str[0] = '\0';
  return buffer;
}

/**
 * @brief Emptys the given string_buffer
 * 
 * @param buffer 
 */
void string_buffer_clear(struct string_buffer *buffer) {
  if (buffer != NULL) {
    buffer->size = 0;
    if (buffer->str != NULL) {
      buffer->str[0] = '\0';
    }
  }
}

/**
 * @brief Deallocates the string_buffer from memory
 * 
 * @param buffer 
 */
void string_buffer_destroy(struct string_buffer *buffer) {
  if (buffer != NULL) {
    if (buffer->str != NULL) {
      free(buffer->str);
      buffer->str = NULL;
    }
    free(buffer);
    buffer = NULL;
  }
}
