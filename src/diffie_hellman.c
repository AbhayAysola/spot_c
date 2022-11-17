#include <openssl/bn.h>
#include "diffie_hellman.h"

BIGNUM* powm(BIGNUM* base, BIGNUM* exp, BIGNUM* modulus);

static const uint8_t DH_PRIME_BYTES[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f, 0xda, 0xa2, 0x21, 0x68, 0xc2,
    0x34, 0xc4, 0xc6, 0x62, 0x8b, 0x80, 0xdc, 0x1c, 0xd1, 0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67,
    0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6, 0x3b, 0x13, 0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79, 0x8e,
    0x34, 0x04, 0xdd, 0xef, 0x95, 0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b, 0x30, 0x2b, 0x0a, 0x6d,
    0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d, 0x6d, 0x51, 0xc2, 0x45, 0xe4, 0x85, 0xb5,
    0x76, 0x62, 0x5e, 0x7e, 0xc6, 0xf4, 0x4c, 0x42, 0xe9, 0xa6, 0x3a, 0x36, 0x20, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

struct DhLocalKeys* dh_random() {
    BIGNUM* DH_PRIME = NULL;
    DH_PRIME = BN_bin2bn(DH_PRIME_BYTES, DH_PRIME_SIZE_IN_BYTES, DH_PRIME);

    BIGNUM* DH_GEN = NULL;
    BN_dec2bn(&DH_GEN, "2");

    BIGNUM* private_key = BN_new();                      // Random private key
    BN_rand(private_key, PRIV_KEY_SIZE_IN_BYTES * 8, 1, 0);

    // Convert BIGNUMs to byte arrays
    BIGNUM* public_key = powm(DH_GEN, private_key, DH_PRIME);
    uint8_t* pub = malloc(BN_num_bytes(public_key));
    BN_bn2bin(public_key, pub);
    uint8_t* priv = malloc(BN_num_bytes(private_key));
    BN_bn2bin(private_key, priv);

    struct DhLocalKeys* dh = malloc(sizeof(struct DhLocalKeys));
    dh->private_key = priv;
    dh->public_key = pub;

    // Cleanup
    BN_free(DH_PRIME);
    BN_free(DH_GEN);
    BN_free(private_key);
    BN_free(public_key);
    return dh;
}

uint8_t*
shared_secret(struct DhLocalKeys* dh, uint8_t* remote_key, size_t remote_key_len) {
    BIGNUM* remote_key_bn = NULL;
    remote_key_bn = BN_bin2bn(remote_key, remote_key_len, remote_key_bn);
    BIGNUM* private_key = NULL;
    private_key = BN_bin2bn(dh->private_key, 32, private_key);
    BIGNUM* DH_PRIME = NULL;
    DH_PRIME = BN_bin2bn(DH_PRIME_BYTES, DH_PRIME_SIZE_IN_BYTES, DH_PRIME);
    BIGNUM* shared_key_bn = powm(remote_key_bn, private_key, DH_PRIME);
    uint8_t* shared_key = malloc(BN_num_bytes(shared_key_bn));
    BN_bn2bin(shared_key_bn, shared_key);

    BN_free(remote_key_bn);
    BN_free(private_key);
    BN_free(DH_PRIME);
    return shared_key;
}

BIGNUM*
powm(BIGNUM* base, BIGNUM* exp, BIGNUM* modulus) {
    BIGNUM* res = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    BN_one(res);

    while (!BN_is_zero(exp)) {
        if (BN_is_odd(exp)) {
            BN_mod_mul(res, res, base, modulus, ctx);
        }
        BN_rshift(exp, exp, 1);
        BN_mod_mul(base, base, base, modulus, ctx);
    }
    BN_CTX_free(ctx);
    return res;
}

void free_dh(struct DhLocalKeys *dh) {
    free(dh->private_key);
    free(dh->public_key);
    free(dh);
}