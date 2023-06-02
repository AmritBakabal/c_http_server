#ifndef __DIE__H

/// prints the error msg and kills the program;
#define DIE(msg, ...) log_die(__FILE__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__);

/// prints error message
#define ERROR(msg, ...) log_error(__FILE__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__);

/// prints warning message
#define WARN(msg, ...) log_warn(__FILE__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__);

/// prints info message
#define INFO(msg, ...) log_info(__FILE__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__);

int log_die(const char *filename, int line, const char *msg, ...);

int log_error(const char *filename, int line, const char *msg, ...);

int log_warn(const char *filename, int line, const char *msg, ...);

int log_info(const char *filename, int line, const char *msg, ...);

#endif