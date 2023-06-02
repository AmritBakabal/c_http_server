#define _XOPEN_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../include/die/die.h"
#include "../include/stringbuffer/stringbuffer.h"

const char *favicon_svg = "";

void get_route(char *reqbuffer, int reqbuffersize, char *route_string,
               int route_size);
void write_web_page_to(struct string_buffer *webpage);
int write_file_to(struct string_buffer *webpage, const char *filename);
void write_favicon_to(struct string_buffer *webpage);

DIR *dir_stream;
struct dirent *dir_entry;

int main() {
  int reuse_address = 1;
  // create socket:
  int serverSocket;
  int port = 8000;
  int RETVAL;

  char display_name[100];
  // buffer for snprintf:
  int buffer_size = 1024;
  char respbuffer[1024];
  char reqbuffer[1024];
  int route_size = 1024;
  char route_string[route_size];
  struct string_buffer *webpage = string_buffer_create();
  int reqbuffersize;
  // server response protocol: rfc 7230
  char *msg =
      "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
      "<h1>Welcome to Nepathya College. </h1>%d";
  int buflen = strlen(msg);

  // file descriptor for the new socket is returned
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  serverSocket != -1 || DIE("socket error");

  // reuse already bound address for this socket
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse_address,
             sizeof(reuse_address)) == 0 ||
      DIE("setsockopt error");

  // ip :
  struct sockaddr_in Internet_address = {
      // man 7 ip (linux):
      .sin_family = AF_INET, /* address family: AF_INET */
      // host to network order (short = 2 bit)
      .sin_port = htons(port), /* port in network byte order */
      // ifconfig inet address:
      .sin_addr.s_addr = htonl(INADDR_ANY) /* internet address */
  };

  // bind:
  bind(serverSocket, (struct sockaddr *)&Internet_address,
       sizeof(struct sockaddr_in)) != -1 ||
      DIE("bind error");

  // listen:
  listen(serverSocket, 10) != -1 || DIE("listen error");

  printf("Listening on %d\n", port);

  // accept:
  int client_socket;
  int client_count = 0;
  ssize_t byte_status;

  while (1) {
    client_socket = accept(serverSocket, NULL, NULL);
    client_socket != -1 || DIE("accept error");

    // read client request
    RETVAL = recv(client_socket, reqbuffer, 1024, 0);
    RETVAL != -1 || DIE("recv error");
    reqbuffer[RETVAL] = '\0';
    // printf("%s\n", reqbuffer);
    char final_route[1024];
    char *hex_space = "%20";
    int idx = 0;
    get_route(reqbuffer, RETVAL, route_string, route_size);
    printf("ROUTE:%s\n", route_string);
    int r_len = strlen(route_string);

    if (strcmp(route_string, "/favicon.ico") == 0) {
      write_file_to(webpage, "images/favicon.svg");
    } else {
      write_web_page_to(webpage);
    }
    // send:
    byte_status = send(client_socket, webpage->str, webpage->size, 0);
    byte_status != -1 || DIE("send error");

    client_count = client_count + 1;
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket) == 0 || DIE("close error");
  }
  close(serverSocket);
  return 0;
}

/**
 * @brief Get the route object
 *
 * @param reqbuffer
 * @param reqbuffersize
 * @param route_string
 * @param route_size
 */
void get_route(char *reqbuffer, int reqbuffersize, char *route_string,
               int route_size) {
  char *method = strtok(reqbuffer, " ");
  char *route = strtok(NULL, " ");
  char *version = strtok(NULL, "\r");
  strncpy(route_string, route, route_size);
}

/*
char *strreplace(const char *src, const char *pattern, const char *replacement)
{ int src_len = strlen(src); int pattern_len = strlen(src); int repl_len =
strlen(replacement); int pattern_idx = 0; for(int i = 0; i < src_len; i ++) {

    }
}
*/

void write_web_page_to(struct string_buffer *webpage) {
  string_buffer_clear(webpage);
  string_buffer_write(webpage,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "\r\n");

  string_buffer_write(webpage,
                      "<html>"
                      "<head>"
                      "</head>"
                      "<body>"
                      "<ul>");
  // add directory information
  dir_stream = opendir(".");
  if (dir_stream != NULL) {
    while ((dir_entry = readdir(dir_stream)) != NULL) {
      if (dir_entry->d_type == DT_DIR) {
        string_buffer_write(webpage, "<li>Directory:%s</li>",
                            dir_entry->d_name);
      } else {
        string_buffer_write(webpage, "<li>File:%s</li>", dir_entry->d_name);
      }
    }
  }
  string_buffer_write(webpage,
                      "</ul>"
                      "</body>"
                      "</html>");
}

int write_file_to(struct string_buffer *webpage, const char *filename) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    ERROR("could not open '%s'", filename);
    return -1;
  }
  struct stat stat_buf;
  if (fstat(fd, &stat_buf) != 0) {
    ERROR("could not get file stat for '%s'", filename);
    return -1;
  }
  int remaining_capacity = webpage->capacity - webpage->size;
  if (remaining_capacity < stat_buf.st_size) {
    int new_capacity = stat_buf.st_size - remaining_capacity;
    webpage->str =
        (uint8_t *)realloc(webpage->str, sizeof(uint8_t) * new_capacity);
    if (webpage->str != NULL) {
      webpage->capacity = new_capacity;
    } else {
      ERROR("could not realloc string_buffer for '%s'", filename);
      return -1;
    }
  }

  string_buffer_clear(webpage);
  string_buffer_write(webpage, "HTTP/1.1 200 OK\r\n");
  string_buffer_write(webpage, "Content-Type: image/svg+xml\r\n");
  string_buffer_write(webpage, "Content-Length: %d\r\n", stat_buf.st_size);
  string_buffer_write(webpage, "Cache-Control: max-age=3600\r\n");
  string_buffer_write(webpage, "\r\n");

  ssize_t available_bytes = stat_buf.st_size;
  ssize_t bytes_read;
  int original_webpage_size = webpage->size;
  do {
    bytes_read = read(fd, webpage->str + webpage->size, available_bytes);
    if (bytes_read < 0) {
      webpage->size = original_webpage_size;
      ERROR("could not read from file to string_buffer for '%s'", filename);
      return -1;
    }
    webpage->size += bytes_read;
    if (available_bytes == bytes_read) break;
    available_bytes -= bytes_read;
  } while (available_bytes != 0);
  return 0;
}
/*
void write_favicon_to(struct string_buffer *webpage) {
  
  
}
*/