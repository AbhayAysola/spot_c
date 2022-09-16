#ifndef DIFFIE_HELLMAN
#define DIFFIE_HELLMAN
struct DhLocalKeys {
    unsigned char *private_key;
    unsigned char *public_key;
};
struct DhLocalKeys *dh_random();

#endif