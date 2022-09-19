#ifndef DIFFIE_HELLMAN
#define DIFFIE_HELLMAN
struct DhLocalKeys {
    uint8_t* private_key;
    uint8_t* public_key;
};
struct DhLocalKeys* dh_random();
uint8_t* shared_secret(struct DhLocalKeys* dh, uint8_t* remote_key, size_t remote_key_len);

#define DH_PRIME_SIZE_IN_BYTES    97
#define PRIV_KEY_SIZE_IN_BYTES    95
#define PUB_KEY_SIZE_IN_BYTES     96
#define SHARED_KEY_SIZE_IN_BYTES  97

#endif