#define _XOPEN_SOURCE
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/die/die.h"
#include "../include/stringbuffer/stringbuffer.h"

void get_route(char* reqbuffer, int reqbuffersize, char* route_string, int route_size);

int main()
{
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
    int reqbuffersize;
    struct string_buffer* web_page = string_buffer_create();

    DIR* dir;
    struct dirent* dir_pointer;
    // server response protocol: rfc 7230

    // file descriptor for the new socket is returned
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverSocket != -1 || DIE("socket error");

    // reuse already bound address for this socket
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address)) == 0 || DIE("setsockopt error");

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
    bind(serverSocket, (struct sockaddr*)&Internet_address, sizeof(struct sockaddr_in)) != -1 || DIE("bind error");

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
        char* hex_space = "%20";
        int idx = 0;
        get_route(reqbuffer, RETVAL, route_string, route_size);
        printf("ROUTE:%s\n", route_string);
        int r_len = strlen(route_string);

        string_buffer_write(web_page, "HTTP/1.1 200 OK\r\n");
        string_buffer_write(web_page, "Content-Type: text/html\r\n");
        string_buffer_write(web_page, "\r\n");
        string_buffer_write(web_page,
            // s/(^|$)/"/g
            "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "<head>"
            "<meta charset=\"UTF-8\" />"
            "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"
            "<title>Document</title>"
            "<style>"
            ".box {"
            "display: flex;"
            "gap: 1rem;"
            "text-align: center;"
            "flex-wrap: wrap;"
            "align-content: flex-start;"
            "justify-content: flex-start;"
            "margin: 1rem;"
            "}"
            ".files {"
            "display: flex;"
            "/* align-items: center; */"
            "flex-direction: column;"
            "padding: 1rem 2.1rem;"
            "max-width: 5rem;"
            "min-width: 5rem;"
            "background-color: #ffffff;"
            "border-radius: 10%%;"
            "font-family: sans-serif;"
            "cursor: pointer;"
            "transition: all ease-in-out 0.3s;"
            "position: relative;"
            "}"
            ".heading {"
            "font-size: 3rem;"
            "}"
            ".files:hover {"
            "/* background-color: #00BFFF; */"
            "box-shadow: 2px 2px 8px 4px #afaaaa;"
            "}"
            "a,a:active {"
            "text-decoration: none;"
            "color: black;"
            "}"
            "body {"
            "background-color: #f0f0f0;"
            "}"
            ".file_name {"
            "text-align: center;"
            "overflow: hidden;"
            "white-space: nowrap;"
            "text-overflow: ellipsis;"
            "}"
            ".file_name::after {"
            "content: attr(data-fileName);"
            "position: absolute;"
            "bottom: 0px;"
            "left: 0;"
            "background-color: #000;"
            "color: #fff;"
            "padding: 5px;"
            "border-radius: 4px;"
            "opacity: 0;"
            "transition: opacity 0.3s;"
            "z-index: 1;"
            "}"
            ".file_name:hover::after {"
            "opacity: 1;"
            "}"
            "</style>"
            "</head>");

        string_buffer_write(web_page,
            "<body>"
            "<div class=\"box\">");
        dir = opendir(".");
        if (dir != NULL) {
            while ((dir_pointer = readdir(dir)) != NULL) {
                if (dir_pointer->d_type == DT_DIR) {

                    if (dir_pointer->d_name[0] != '.') {
                        string_buffer_write(web_page,
                            "<div class=\"files\">"
                            "<a href=\"%1$s\">"
                            "<div class=\"heading\"><b>D</b></div>"
                            "<div class=\"file_name\" data-fileName=\"%1$s\">"
                            "%1$s"
                            "</div>"
                            "</a>"
                            "</div>",
                            dir_pointer->d_name);
                    }
                }
            }
            rewinddir(dir); //points to beggining of directory.
            while ((dir_pointer = readdir(dir)) != NULL) {
                if (dir_pointer->d_type == DT_REG && dir_pointer->d_name[0] != '.') {
                    string_buffer_write(web_page,
                        "<div class=\"files\">"
                        "<a href=\"%1$s\">"
                        "<div class=\"heading\"><b>F</b></div>"
                        "<div class=\"file_name\" data-fileName=\"%1$s\">"
                        "%1$s"
                        "</div>"
                        "</a>"
                        "</div>",
                        dir_pointer->d_name);
                }
            }
        }
        closedir(dir);
        string_buffer_write(web_page,
            "</div>"
            "</body>"
            "</html>");

        // snprintf(respbuffer, buffer_size, msg, client_count);
        // send:
        byte_status = send(client_socket, web_page->str, web_page->size, 0);
        byte_status != -1 || DIE("send error");

        client_count = client_count + 1;
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket) == 0 || DIE("close error");
        string_buffer_clear(web_page);
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
void get_route(char* reqbuffer, int reqbuffersize, char* route_string, int route_size)
{
    char* method = strtok(reqbuffer, " ");
    char* route = strtok(NULL, " ");
    char* version = strtok(NULL, "\r");
    strncpy(route_string, route, route_size);
}

/*
char *strreplace(const char *src, const char *pattern, const char *replacement) {
    int src_len = strlen(src);
    int pattern_len = strlen(src);
    int repl_len = strlen(replacement);
    int pattern_idx = 0;
    for(int i = 0; i < src_len; i ++) {

    }
}
*/
