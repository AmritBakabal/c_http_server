#ifndef __DIE__H
// exits the program with a message
#define DIE(msg) die_func(__FILE__, __LINE__, msg);
// #define INFO(msg

int die_func(const char *filename, int line, const char *msg);

#endif