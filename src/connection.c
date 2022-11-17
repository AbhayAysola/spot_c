#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include "connection.h"
#include "keyexchange.pb-c.h"
#include "diffie_hellman.h"

typedef struct {
    size_t len;
    uint8_t* data;
} Accumulator;

Accumulator client_hello(int sockfd, struct DhLocalKeys* dh);
size_t recv_packet(int sockfd, Accumulator* acc);
Accumulator* read_into_acc(int sockfd, size_t size, Accumulator* acc);
uint8_t** compute_keys(uint8_t* shared_key, Accumulator* packets);
void client_response(int sockfd, uint8_t *challenge);

void handshake(int sockfd) {
    struct DhLocalKeys* dh = dh_random();
    Accumulator accumulator = client_hello(sockfd, dh);
    size_t client_hello_sent_bytes = accumulator.len;
    size_t ap_response_recvd_bytes = recv_packet(sockfd, &accumulator);
    if (ap_response_recvd_bytes == -1) {
        printf("Error recieving APResponse");
    }

    APResponseMessage* ap_response;
    ap_response = apresponse_message__unpack(NULL, ap_response_recvd_bytes, accumulator.data + client_hello_sent_bytes + 4);
    if (ap_response == NULL) {
        return;
    }
    if (ap_response_recvd_bytes == -1)
        return;
    printf("recv: %i bytes\n", ap_response_recvd_bytes);

    uint8_t* shared_key = shared_secret(dh, ap_response->challenge->login_crypto_challenge->diffie_hellman->gs.data, ap_response->challenge->login_crypto_challenge->diffie_hellman->gs.len);
    uint8_t **keys = compute_keys(shared_key, &accumulator);
    client_response(sockfd, keys[0]);

    free(shared_key); 
    free(keys[0]);
    free(keys[1]-20);       // -20 because we return data + 20 in the compute_keys function
    free(keys);             // free(keys[2]) isn't necessary because it was allocated at the same time as keys[1]
    free(accumulator.data);
    // apresponse_message__free_unpacked(ap_response, NULL); // Commented because it is causing pointer issues
    free_dh(dh);
}  

Accumulator
client_hello(int sockfd, struct DhLocalKeys* dh) {
    // Construct build_info
    BuildInfo build_info = BUILD_INFO__INIT;
    build_info.product = PRODUCT__PRODUCT_CLIENT;
    build_info.product_flags = PRODUCT_FLAGS__PRODUCT_FLAG_NONE;
    build_info.platform = PLATFORM__PLATFORM_LINUX_X86;
    build_info.version = (uint64_t)115800820;

    // DiffieHellman Encryption
    Cryptosuite cryptosuites_supported = CRYPTOSUITE__CRYPTO_SUITE_SHANNON;
    LoginCryptoHelloUnion login_crypto_hello = LOGIN_CRYPTO_HELLO_UNION__INIT;
    LoginCryptoDiffieHellmanHello diffie_hellman = LOGIN_CRYPTO_DIFFIE_HELLMAN_HELLO__INIT;
    diffie_hellman.gc.len = PUB_KEY_SIZE_IN_BYTES;
    diffie_hellman.gc.data = dh->public_key;
    diffie_hellman.server_keys_known = 1;
    login_crypto_hello.diffie_hellman = &diffie_hellman;

    // Building the ClientHello message
    ClientHello msg = CLIENT_HELLO__INIT;
    msg.build_info = &build_info;
    msg.cryptosuites_supported = &cryptosuites_supported;
    msg.login_crypto_hello = &login_crypto_hello;

    uint8_t nonce[16];
    RAND_bytes(nonce, 16);
    msg.client_nonce.len = 16;
    msg.client_nonce.data = nonce;

    uint8_t padding = 0x1e;
    msg.padding.data = &padding;
    msg.padding.len = 1;

    size_t len = 2 + 4 + client_hello__get_packed_size(&msg);
    uint8_t* client_hello = malloc(len);
    client_hello[0] = 0;
    client_hello[1] = 4;
    uint32_t len_bytes = htonl((uint32_t)len);
    memcpy(client_hello + 2, &len_bytes, 4);
    client_hello__pack(&msg, client_hello + 6);

    printf("len: %i, sent: %i\n", len, send(sockfd, client_hello, len, 0));
    return (Accumulator) {
        .len = len, .data = client_hello
    };
}

size_t
recv_packet(int sockfd, Accumulator* acc) {
    Accumulator* header = read_into_acc(sockfd, 4, acc);
    if (header == NULL) {
        return -1;
    }
    uint32_t size;
    memcpy(&size, header->data + header->len - 4, 4);
    Accumulator* data = read_into_acc(sockfd, ((size_t)ntohl(size)) - 4, acc);
    return ((size_t)ntohl(size)) - 4;
}

Accumulator*
read_into_acc(int sockfd, size_t size, Accumulator* acc) {
    size_t offset = acc->len;
    acc->data = realloc(acc->data, offset + size);
    if (acc->data == NULL) {
        return NULL;
    }

    size_t bytes_read = recv(sockfd, acc->data + offset, size, 0);
    if (bytes_read == -1) {
        return NULL;
    }

    acc->len += bytes_read;
    return acc;
}

uint8_t**
compute_keys(uint8_t* shared_key, Accumulator* packets) {
    const EVP_MD* sha1 = EVP_sha1();
    uint8_t* data = malloc(100);
    for (int i = 1; i < 6; i++) {
        HMAC_CTX* ctx = HMAC_CTX_new();
        int err = HMAC_Init_ex(ctx, shared_key, SHARED_KEY_SIZE_IN_BYTES, sha1, NULL);
        if (!err) {
            printf("%s\n", "Error initializing HMAC");
        }
        HMAC_Update(ctx, packets->data, packets->len);
        const unsigned char j[1] = {i};
        HMAC_Update(ctx, j, 1);
        unsigned int len;
        err = HMAC_Final(ctx, data+(i*20), &len);
        if (!err) {
            printf("%s\n", "Error finalizing HMAC");
        }
        HMAC_CTX_free(ctx);
    }

    HMAC_CTX* ctx = HMAC_CTX_new();
    int err = HMAC_Init_ex(ctx, data, 20, sha1, NULL);
    if (!err) {
        printf("%s\n", "Error initializing HMAC");
    }
    HMAC_Update(ctx, packets->data, packets->len);
    unsigned int len;
    uint8_t *challenge = malloc(20);
    if (challenge == NULL)
        printf("%s", "malloc failed!");
    HMAC_Final(ctx, challenge, &len);
    HMAC_CTX_free(ctx);

    uint8_t ** keys = malloc(3 * sizeof(uint8_t *));
    keys[0] = challenge;
    keys[1] = data+20;
    keys[2] = data+52;
    return keys;
}

void client_response(int sockfd, uint8_t *challenge) {
    ClientResponsePlaintext msg = CLIENT_RESPONSE_PLAINTEXT__INIT;
    LoginCryptoResponseUnion login_crypto_response = LOGIN_CRYPTO_RESPONSE_UNION__INIT;
    LoginCryptoDiffieHellmanResponse diffie_hellman = LOGIN_CRYPTO_DIFFIE_HELLMAN_RESPONSE__INIT;
    diffie_hellman.hmac.data = challenge;
    diffie_hellman.hmac.len  = 20;
    login_crypto_response.diffie_hellman = &diffie_hellman;
    msg.login_crypto_response = &login_crypto_response;
    CryptoResponseUnion crypto_response = CRYPTO_RESPONSE_UNION__INIT;
    PoWResponseUnion pow_response = PO_WRESPONSE_UNION__INIT;
    msg.crypto_response = &crypto_response;
    msg.pow_response = &pow_response;
    size_t len = 4 + client_response_plaintext__get_packed_size(&msg);
    uint8_t* client_response = malloc(len);
    uint32_t len_bytes = htonl((uint32_t)len);
    memcpy(client_response, &len_bytes, 4);
    client_response_plaintext__pack(&msg, client_response + 4);
    printf("len: %i, sent: %i\n", len, send(sockfd, client_response, len, 0));
    free(client_response);
}