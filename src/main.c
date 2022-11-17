#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "ap.h"
#include "connection.h"

int main(void) {
    // Initialize socket data
    struct addrinfo hints, *res;
    int sockfd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char* ap = get_ap(NULL, 0);

    // Split ap into port and ap on ':' character
    char* port = malloc(5);
    for (int i = 0; ap[i - 1] != ':'; i++) {
        port = &ap[i + 1];
    }
    *(port - 1) = '\0';

    int err;
    if ((err = getaddrinfo(ap, port, &hints, &res))) {
        printf("%s", gai_strerror(err));
    }
    char ip_addr[INET_ADDRSTRLEN];
    inet_ntop(res->ai_addr->sa_family, res->ai_addr, ip_addr, INET_ADDRSTRLEN);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if ((err = connect(sockfd, res->ai_addr, res->ai_addrlen))) {
        printf("%s", gai_strerror(err));
    }
    handshake(sockfd);
    freeaddrinfo(res);
}
