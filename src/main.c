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
void write_web_page_to(struct string_buffer *webpage, const char *route);
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

    write_web_page_to(webpage, route_string);
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

char *strreplace(const char *src, const char *pattern,
                 const char *replacement) {
  int src_len = strlen(src);
  int pattern_len = strlen(src);
  int repl_len = strlen(replacement);
  int pattern_idx = 0;
  for (int i = 0; i < src_len; i++) {
    
  }
}

void write_web_page_to(struct string_buffer *webpage, const char *route) {
  DIR *dir_stream;
  struct dirent *dir_entry;
  string_buffer_clear(webpage);

  // since home page is '/', we donot want another backslash
  char *optional_slash = "/";
  if (strcmp(route, "/") == 0) {
    optional_slash = "";
  }

  // prepend route with "."; to enforce relative path from current directory
  // !IMPORTANT for security
  char file_path[1024];
  snprintf(file_path, 1024, ".%s", route);

  // check for the file type of route
  struct stat stat_buf;
  int stat_status = stat(file_path, &stat_buf);
  if (stat_status == 0) {
    if (S_ISDIR(stat_buf.st_mode)) {
      // this is directory
      string_buffer_write(webpage,
                          "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "\r\n");
      string_buffer_write(
          webpage,
          "<!DOCTYPE html>"
          "<html lang=\"en\">"
          "<head>"
          "<meta charset=\"UTF-8\" />"
          "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />"
          "<meta name=\"viewport\" content=\"width=device-width, "
          "initial-scale=1.0\" />"
          "<title>Local share</title>"
          "</head>"
          "<style>"
          "* {"
          "margin: 0;"
          "padding: 0;"
          "}"
          "#box {"
          "display: flex;"
          "flex-wrap: wrap;"
          "}"
          ".files {"
          "position: relative;"
          "display: flex;"
          "flex-direction: column;"
          "align-items: center;"
          "color: inherit;"
          "margin: 16px;"
          "padding: 16px;"
          "text-decoration: none;"
          "transition: ease-in-out 0.25s;"
          "width: 100px;"
          "box-shadow: 0 2px 2px 0px rgba(0, 0, 0, 0.3),"
          "0 3px 1px -2px rgba(0, 0, 0, 0.4);"
          "}"
          ".files::after {"
          "content: attr(data-filename);"
          "position: absolute;"
          "left: 4px;"
          "bottom: 4px;"
          "color: white;"
          "border-radius: 4px;"
          "background-color: #ba3d0d;"
          "padding: 4px;"
          "opacity:0;"
          "transition: ease-in-out 0.25s;"
          "}"
          ".files:hover::after {"
          "opacity:1;"
          "}"
          "p {"
          "width: inherit;"
          "text-overflow: ellipsis;"
          "white-space: nowrap;"
          "overflow: hidden;"
          "}"
          ".files:hover {"
          "text-decoration: none;"
          "box-shadow: 0 8px 10px 1px rgba(0, 0, 0, 0.3),"
          "0 3px 14px 2px rgba(0, 0, 0, 0.4);"
          "}"
          "</style>"
          "<body>"
          "<svg style=\"display: none\">"
          "<defs>"
          "<symbol id=\"file-icon-text\" viewBox=\"0 0 210 297\">"
          "<path d=\"M 7.8269535e-7,3.6519676e-7 V 296.99999 H 210 V "
          "3.6519676e-7 Z"
          "M 23.55052,42.369852 h 162.90613 v 8.377459 H 23.55052 Z m "
          "0,33.979391 "
          "h 162.90613 v 8.375293 H 23.55052 Z m 0,33.985877 h 96.24049 v "
          "8.37533 "
          "H 23.55052 Z m 0,33.97726 h 162.90613 v 8.37528 H 23.55052 Z m "
          "0,33.97721 "
          "h 134.49936 v 8.3753 H 23.55052 Z m 0,33.98592 H 143.7546 v 8.37518 "
          "H 23.55052 Z m 0,33.97722 h 152.0198 v 8.37523 H 23.55052 Z\""
          " style=\"opacity: 1; fill: #dd4814; stroke-linejoin: round\""
          "/>"
          "</symbol>"
          "<symbol id=\"file-icon-binary\" viewBox=\"0 0 210 297\">"
          "<path d=\"M -0.00263557,-2.6174098e-7 C -0.00263557,99.000013 "
          "-0.00263557,197.99999 -0.00263557,297 69.99737,297 139.99737,297 "
          "209.99737,297 c 0,-69.25685 0,-138.51373 0,-207.770579 C "
          "180.7069,59.486293 151.41642,29.743127 122.12594,-2.6174109e-7 c "
          "-40.709522,0 -81.41905,0 -122.12857557,1.1e-13 z\""
          " style=\"fill: #dd4814; stroke-linejoin: round\""
          "/>"
          "<path d=\"M 122.13076,-2.6174098e-7 V 89.229421 H 210 Z\""
          " style=\"fill: #e46d43; stroke-linejoin: round\""
          "/>"
          "</symbol>"
          "<symbol id=\"file-icon-video\" viewBox=\"0 0 210 297\">"
          "<path d=\"M 0,0 V 297 H 210 V 0 Z M 7.75855,22.24261 H 21.47639 V "
          "41.19802 H 7.75855 Z m 180.76366,0 h 13.71924 V 41.19802 H "
          "188.52221 "
          "Z M 29.23494,42.04823 h 151.53008 v 86.81171 H 29.23494 Z M "
          "7.75855,80.4175 H 21.47639 V 99.37286 H 7.75855 Z m 180.76366,0 h "
          "13.71924 V 99.37286 H 188.52221 Z M 7.75855,138.59233 h 13.71789 v "
          "18.95344 H 7.75855 Z m 180.76366,0 h 13.71924 v 18.95344 H "
          "188.52221 "
          "Z M 29.23494,168.14002 h 151.53008 v 86.81175 H 29.23494 Z M "
          "7.75855,196.76515 h 13.71784 v 18.95536 H 7.75855 Z m 180.76366,0 h "
          "13.71924 v 18.95536 H 188.52221 Z M 7.75855,254.94003 h 13.71789 v "
          "18.95344 H 7.75855 Z m 180.76366,0 h 13.71924 v 18.95344 h "
          "-13.71924 "
          "z\""
          " style=\"fill: #dd4814; stroke-linejoin: round\""
          "/>"
          "</symbol>"
          "<symbol id=\"file-icon-audio\" viewBox=\"0 0 210 297\">"
          "<path d=\"M 58.20345,51.482495 V 178.06584 c -6.30597,-4.89505 "
          "-13.98549,-7.54246 -21.87928,-7.54259 -20.088158,-2.1e-4 "
          "-36.373195,16.78708 -36.37417,37.49603 -2.11e-4,20.70979 "
          "16.285165,37.49843 36.37417,37.49822 20.08897,2.1e-4 "
          "36.37439,-16.78843 36.37417,-37.49822 V 97.027034 h 122.8568 v "
          "62.837206 c -6.30601,-4.89505 -13.98552,-7.54246 -21.87931,-7.54258 "
          "-20.08815,-2.1e-4 -36.37357,16.78667 -36.37417,37.49602 "
          "-6e-4,20.70935 17.83194,37.01815 38.02544,34.89665 20.19349,-2.1215 "
          "33.45967,-17.43154 34.7229,-33.04824 V 51.482495 Z m "
          "14.49489,14.94281 h 122.8568 v 23.1314 H 72.69834 Z\""
          " style=\"fill: #dd4814; stroke-linejoin: round\""
          "/>"
          "</symbol>"
          "<symbol id=\"file-icon-folder\" viewBox=\"0 0 210 297\">"
          "<path d=\"M 10.788993,78.629395 H 199.211 c 5.9771,0 "
          "10.78899,4.811893 10.78899,10.788998 V 230.5967 c 0,5.9771 "
          "-4.81189,10.789 -10.78899,10.789 H 10.788993 C 4.8118879,241.3857 "
          "-5.4607431e-6,236.5738 -5.4607431e-6,230.5967 V 89.418393 C "
          "-5.4607431e-6,83.441288 4.8118879,78.629395 10.788993,78.629395 Z\""
          " style=\"fill: #ba3d0d; stroke-linejoin: round\""
          "/>"
          "<path d=\"m 33.459324,55.614296 c 0,42.52275 0,85.045484 "
          "0,127.568234 "
          "34.999118,0 69.998236,0 104.997356,0 0,-29.74739 0,-59.49479 "
          "0,-89.242175 -14.64487,-12.775347 -29.28974,-25.550711 "
          "-43.934607,-38.326059 -20.35425,0 -40.7085,0 -61.062749,0 z\""
          " style=\"fill: #e05a2c; stroke-linejoin: round\""
          "/>"
          "<path d=\"M 94.524701,55.614296 V 93.940355 H 138.45932 Z\""
          " style=\"fill: #e77f5b; stroke-linejoin: round\""
          "/>"
          "<path d=\"M 10.788993,103.70753 H 199.211 c 5.9771,0 "
          "10.78899,4.81189 "
          "10.78899,10.789 v 116.10018 c 0,5.9771 -4.81189,10.78899 "
          "-10.78899,10.78899 H 10.788993 C 4.8118879,241.3857 "
          "-5.4607431e-6,236.57381 -5.4607431e-6,230.59671 V 114.49653 c "
          "0,-5.97711 4.8118933607431,-10.789 10.7889984607431,-10.789 z\""
          " style=\"fill: #dd4814; stroke-linejoin: round\""
          "/>"
          "</symbol>"
          "<symbol id=\"file-icon-image\" viewBox=\"0 0 210 297\">"
          "<path d=\"M -0.00263557,-2.6174098e-7 C -0.00263557,99.000013 "
          "-0.00263557,197.99999 -0.00263557,297 69.99737,297 139.99737,297 "
          "209.99737,297 c 0,-69.25685 0,-138.51373 0,-207.770579 C "
          "180.7069,59.486293 151.41642,29.743127 122.12594,-2.6174109e-7 c "
          "-40.709522,0 -81.41905,0 -122.12857557,1.1e-13 z\""
          " style=\"fill: #dd4814; stroke-linejoin: round\""
          "/>"
          "<path d=\"M 122.13076,-2.6174098e-7 V 89.229421 H 210 Z\""
          " style=\"fill: #e46d43; stroke-linejoin: round\""
          "/>"
          "<path d=\"m 32.504249,228.952 34.156565,-36.05377 "
          "28.111716,23.83193 "
          "40.26726,-51.18868 45.56648,63.41052\""
          " style=\""
          "fill: none;"
          "stroke: #ffffff;"
          "stroke-width: 5;"
          "stroke-linecap: butt;"
          "stroke-linejoin: miter;"
          "stroke-opacity: 1;"
          "stroke-dasharray: none;"
          "\""
          "/>"
          "<circle cx=\"65.139748\" cy=\"135.35245\" r=\"12.769653\""
          " style=\""
          "opacity: 1;"
          "fill: none;"
          "stroke: #ffffff;"
          "stroke-width: 5;"
          "stroke-linejoin: round;"
          "stroke-dasharray: none;"
          "stop-color: #000000;\" />"
          "<path d=\"m 62.640625,107.77539 v 10.10352 h 5 v -10.10352 z\" "
          "style=\"fill: white\" />"
          "<path d=\"m 83.033203,114.15234 -7.144531,7.14258 3.537109,3.53711 "
          "7.142578,-7.14453 z\""
          " style=\"fill: white\""
          "/>"
          "<path d=\"m 82.841797,133.08008 v 5 h 10.103515 v -5 z\""
          " style=\"fill: white\""
          "/>"
          "<path d=\"m 79.425781,146.33008 -3.537109,3.53515 7.144531,7.14454 "
          "3.535156,-3.53516 z\""
          " style=\"fill: white\""
          "/>"
          "<path d=\"m 62.640625,153.2832 v 10.10352 h 5 V 153.2832 Z\""
          " style=\"fill: white\""
          "/>"
          "<path d=\"m 50.855469,146.33008 -7.144532,7.14453 3.535157,3.53516 "
          "7.144531,-7.14454 z\""
          " style=\"fill: white\""
          "/>"
          "<path d=\"m 37.333984,133.08008 v 5 H 47.4375 v -5 z\""
          " style=\"fill: white\""
          "/>"
          "<path d=\"m 47.246094,114.15234 -3.535157,3.53516 7.144532,7.14453 "
          "3.535156,-3.53711 z\""
          " style=\"fill: white\""
          "/>"
          "</symbol>"
          "</defs>"
          "</svg>"
          "<div id=\"box\">");

      dir_stream = opendir(file_path);
      char extension[1024];
      if (dir_stream != NULL) {
        while ((dir_entry = readdir(dir_stream)) != NULL) {
          if (dir_entry->d_name[0] == '.') {
            continue;
          }
          if (dir_entry->d_type == DT_DIR) {
            string_buffer_write(
                webpage,
                "<a class=\"files\" href=\"%2$s%3$s%1$s\" "
                "data-filename=\"%1$s\">"
                "<svg width=\"100\"><use href=\"#file-icon-folder\"/></svg>"
                "<p>%1$s</p></a>",
                dir_entry->d_name, route, optional_slash);
          } else {
            // extract extension from filename if available
            int s = strlen(dir_entry->d_name);
            int k = 0;
            int has_extension = 0;
            char file_icon_type[20];
            for (int i = s - 1; i >= 0; i--) {
              if (dir_entry->d_name[i] == '.') {
                has_extension = 1;
                break;
              }
              extension[k++] = dir_entry->d_name[i];
            }
            if (has_extension == 0) {
              strcpy(extension, ".noextension");
            } else {
              extension[k] = '\0';
              s = strlen(extension);
              s /= 2;   // go up to first half of string
              char ch;  // for swapping
              for (int i = 0; i < s / 2; i++) {
                ch = extension[i];
                extension[i] = extension[s - 1 - i];
                extension[s - 1 - i] = ch;
              }
            }
            // check for extension
            if (strcmp(extension, "txt") == 0) {
              strcpy(file_icon_type, "text");
            } else if (strcmp(extension, "jpg") == 0) {
              strcpy(file_icon_type, "image");
            } else if (strcmp(extension, "jpeg") == 0) {
              strcpy(file_icon_type, "image");
            } else if (strcmp(extension, "png") == 0) {
              strcpy(file_icon_type, "image");
            } else if (strcmp(extension, "mp4") == 0) {
              strcpy(file_icon_type, "video");
            } else if (strcmp(extension, "mp3") == 0) {
              strcpy(file_icon_type, "audio");
            } else if (strcmp(extension, "wav") == 0) {
              strcpy(file_icon_type, "audio");
            } else if (strcmp(extension, "avi") == 0) {
              strcpy(file_icon_type, "audio");
            } else {
              strcpy(file_icon_type, "binary");
            }
            string_buffer_write(
                webpage,
                "<a class=\"files\" href=\"%2$s%3$s%1$s\" "
                "data-filename=\"%1$s\">"
                "<svg width=\"100\"><use href=\"#file-icon-%4$s\"/></svg>"
                "<p>%1$s</p></a>",
                dir_entry->d_name, route, optional_slash, file_icon_type);
          }
        }
      } else {
        // directory stream is NULL
      }
      string_buffer_write(webpage,
                          "</div>"
                          "</body>"
                          "</html>");
      closedir(dir_stream);
    } else if (S_ISREG(stat_buf.st_mode)) {
      // this is a regular file
      // send file as data
      write_file_to(webpage, file_path);
    } else {
      WARN("%s is not a directory or a regular file.", file_path);
      string_buffer_write(webpage,
                          "HTTP/1.1 404 NOT FOUND\r\n"
                          "\r\n");
    }
  } else if (strcmp(route, "/favicon.ico") == 0) {
    // send favicon
    char *favicon =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 448 "
        "512\"><path d=\"M352 224c53 0 96-43 96-96s-43-96-96-96s-96 43-96 96c0 "
        "4 .2 8 .7 11.9l-94.1 47C145.4 170.2 121.9 160 96 160c-53 0-96 43-96 "
        "96s43 96 96 96c25.9 0 49.4-10.2 66.6-26.9l94.1 47c-.5 3.9-.7 7.8-.7 "
        "11.9c0 53 43 96 96 96s96-43 96-96s-43-96-96-96c-25.9 0-49.4 10.2-66.6 "
        "26.9l-94.1-47c.5-3.9 .7-7.8 .7-11.9s-.2-8-.7-11.9l94.1-47C302.6 213.8 "
        "326.1 224 352 224z\" "
        "style=\"fill:#dd4814;stroke-linejoin:round\"/></svg>";
    string_buffer_write(webpage,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: image/svg+xml\r\n"
                        "Content-Length: %d\r\n"
                        "Cache-Control: max-age=86400\r\n"
                        "\r\n"
                        "%s",
                        strlen(favicon), favicon);
  } else {
    // non existant filepath
    ERROR("could not get stat for %s", file_path);
    string_buffer_write(webpage,
                        "HTTP/1.1 404 NOT FOUND\r\n"
                        "\r\n");
  }
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
  // INFO("filesize: %d", stat_buf.st_size);
  // INFO("remaining capacity before realloc: %d", remaining_capacity);
  if (remaining_capacity < stat_buf.st_size) {
    // int new_capacity = stat_buf.st_size - remaining_capacity;
    int new_capacity = stat_buf.st_size + webpage->capacity;
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
  string_buffer_write(webpage,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/octet-stream\r\n"
                      "Content-Length: %d\r\n"
                      "Content-Disposition: attachment\r\n"
                      "Cache-Control: max-age=3600\r\n"
                      "\r\n",
                      stat_buf.st_size);
  // string_buffer_write(webpage,
  //                     "Content-Disposition: attachment; filename=\"%s\"\r\n",
  //                     filename);
  // INFO("%s", webpage->str);
  // INFO("webpage size: %d", webpage->size);

  ssize_t available_bytes = stat_buf.st_size;
  ssize_t bytes_read;
  int original_webpage_size = webpage->size;
  do {
    bytes_read = read(fd, webpage->str + webpage->size, available_bytes);
    if (bytes_read < 0) {
      webpage->size = original_webpage_size;
      ERROR("could not read %d bytes from '%s' to string_buffer",
            available_bytes, filename);
      return -1;
    }
    webpage->size += bytes_read;
    available_bytes -= bytes_read;
  } while (available_bytes != 0);
  return 0;
}