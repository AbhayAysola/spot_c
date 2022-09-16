// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include "ap.h"

// int
// main(void) {
//     // Initialize socket data
//     struct addrinfo hints, *res;
//     int sockfd;
//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_INET;
//     hints.ai_socktype = SOCK_STREAM;

//     char* ap = get_ap(NULL, 0);

//     // Split ap into port and ap on ':' character
//     char* port = malloc(5);
//     for (int i = 0; ap[i - 1] != ':'; i++) {
//         port = &ap[i + 1];
//     }

//     *(port - 1) = '\0';
//     printf("ap: %s, port: %s\n", ap, port);

//     int err;
//     if (err = getaddrinfo(ap, port, &hints, &res)) {
//         printf("%s", gai_strerror(err));
//     }

//     sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

//     char ip_addr[INET_ADDRSTRLEN];
//     inet_ntop(res->ai_addr->sa_family, res->ai_addr, ip_addr, INET_ADDRSTRLEN);
//     // printf("%i", connect(sockfd, res->ai_addr, res->ai_addrlen));
//     printf("%s\n", ip_addr);

//     freeaddrinfo(res);
// }


#include "keyexchange.pb-c.h"
#include <stdio.h>
#include <stdlib.h>
#include "diffie_hellman.h"

int
main(void) {
    // Construct build_info
    BuildInfo build_info = BUILD_INFO__INIT;
    build_info.product = PRODUCT__PRODUCT_CLIENT;
    build_info.product_flags = PRODUCT_FLAGS__PRODUCT_FLAG_NONE;
    build_info.platform = PLATFORM__PLATFORM_LINUX_X86;
    build_info.version = (uint64_t)115800820;

    Cryptosuite cryptosuites_supported = CRYPTOSUITE__CRYPTO_SUITE_SHANNON;

    LoginCryptoHelloUnion login_crypto_hello = LOGIN_CRYPTO_HELLO_UNION__INIT;
    LoginCryptoDiffieHellmanHello diffie_hellman = LOGIN_CRYPTO_DIFFIE_HELLMAN_HELLO__INIT;

    struct DhLocalKeys *dh = dh_random();

    return 0;
}