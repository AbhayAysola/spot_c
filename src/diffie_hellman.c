#include <openssl/bn.h>
#include "diffie_hellman.h"

BIGNUM * powm(BIGNUM * base, BIGNUM * exp, BIGNUM * modulus);

static const char DH_PRIME_STR[] = "1552518092300708935130918131258481755631334049434514313202351194902966239949102107258669453876591642442910007680288864229150803718918046342632727613031282983744380820890196288509170691316593175367469551763119843371637221007210577919";

struct DhLocalKeys *dh_random() {
    BIGNUM *DH_PRIME = NULL;
    BN_dec2bn(&DH_PRIME, DH_PRIME_STR);

    BIGNUM *DH_GEN = NULL;
    BN_dec2bn(&DH_GEN, "2");

    BIGNUM *private_key = BN_new();
    BN_rand(private_key, 95*8, 1, 0);

    BIGNUM * public_key = powm(DH_GEN, private_key, DH_PRIME);
    unsigned char *pub = malloc(BN_bn2mpi(public_key, NULL));
    BN_bn2mpi(public_key, pub);
    pub += 4;
    *(pub + 96) = '\0';
    unsigned char *priv = malloc(BN_bn2mpi(private_key, NULL));
    BN_bn2mpi(private_key, priv);
    priv += 4;
    *(priv + 95*8) = '\0';
    struct DhLocalKeys *dh = malloc(sizeof(struct DhLocalKeys));
    dh->private_key = priv;
    dh->public_key = pub;
    return dh;
}

BIGNUM * powm(BIGNUM * base, BIGNUM * exp, BIGNUM * modulus) {
    BIGNUM * res = BN_new();
    BN_CTX *ctx = BN_CTX_new();
    BN_one(res);

    while (!BN_is_zero(exp)) {
        if (BN_is_odd(exp))
            BN_mod_mul(res, res, base, modulus, ctx);
        BN_rshift(exp, exp, 1);
        BN_mod_mul(base, base, base, modulus, ctx);
    }

    return res;
}