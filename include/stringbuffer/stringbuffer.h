#ifndef _STRINGBUFFER_H
#include <stdint.h>
#define _STRINGBUFFER_H
#define FMT(X, Y, ...) string_buffer_write(X, Y, __VA_ARGS__); 

struct string_buffer {
  uint8_t *str;
  int size;
  int capacity;
};

/**
 * @brief doubles the string_buffer size
 * 
 * @param buffer 
 * @return uint8_t * 
 */
uint8_t *string_buffer_expand(struct string_buffer *buffer); 

/**
 * @brief Writes the given formatted string to the string_buffer
 * 
 * @param buffer 
 * @param fmt format string 
 * @param ... format specifiers' arguments
 * @return int total number of characters written to the buffer
 */
int string_buffer_write(struct string_buffer *buffer, const char *fmt, ...); 

/**
 * @brief Initializes the given `struct string_buffer *buffer`
 * The string is empty "" by default
 * 
 * @returns pointer to sthe struct
 */
struct string_buffer *string_buffer_create(); 

/**
 * @brief Emptys the given string_buffer
 * 
 * @param buffer 
 */
void string_buffer_clear(struct string_buffer *buffer); 

/**
 * @brief Deallocates the string_buffer from memory
 * 
 * @param buffer 
 */
void string_buffer_destroy(struct string_buffer *buffer); 
#endif