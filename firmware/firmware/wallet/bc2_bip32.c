#include "bc2_bip32.h"
#include "bc2_crypto.h"
#include "bc2_encoding.h"
#include "bc2_compat.h"

#include <string.h>
#include <stdlib.h>

static void be32(uint8_t *p, uint32_t x) {
    p[0] = (uint8_t)(x >> 24);
    p[1] = (uint8_t)(x >> 16);
    p[2] = (uint8_t)(x >> 8);
    p[3] = (uint8_t)x;
}

#ifdef ESP_PLATFORM

#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"
#include "esp_random.h"

static int bc2_ecp_rng(void *context, unsigned char *output, size_t length) {
    (void)context;
    if (output == NULL) return -1;
    esp_fill_random(output, length);
    return 0;
}

bool bc2_secp256k1_public(const uint8_t key[32], uint8_t output[33]) {
    mbedtls_ecp_group group;
    mbedtls_mpi private_key;
    mbedtls_ecp_point public_key;
    size_t written = 0U;
    int result;

    if (key == NULL || output == NULL) return false;

    mbedtls_ecp_group_init(&group);
    mbedtls_mpi_init(&private_key);
    mbedtls_ecp_point_init(&public_key);

    result = mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256K1);
    if (result == 0) result = mbedtls_mpi_read_binary(&private_key, key, 32U);
    if (result == 0 &&
        (mbedtls_mpi_cmp_int(&private_key, 1) < 0 ||
         mbedtls_mpi_cmp_mpi(&private_key, &group.N) >= 0))
        result = -1;
    if (result == 0)
        result = mbedtls_ecp_mul(&group, &public_key, &private_key,
                                 &group.G, bc2_ecp_rng, NULL);
    if (result == 0)
        result = mbedtls_ecp_point_write_binary(
            &group, &public_key, MBEDTLS_ECP_PF_COMPRESSED,
            &written, output, 33U);

    mbedtls_ecp_point_free(&public_key);
    mbedtls_mpi_free(&private_key);
    mbedtls_ecp_group_free(&group);
    return result == 0 && written == 33U;
}

static bool add_private_scalars(const uint8_t left[32], const uint8_t right[32],
                                uint8_t output[32]) {
    mbedtls_ecp_group group;
    mbedtls_mpi a;
    mbedtls_mpi b;
    mbedtls_mpi sum;
    int result;

    mbedtls_ecp_group_init(&group);
    mbedtls_mpi_init(&a);
    mbedtls_mpi_init(&b);
    mbedtls_mpi_init(&sum);

    result = mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256K1);
    if (result == 0) result = mbedtls_mpi_read_binary(&a, left, 32U);
    if (result == 0) result = mbedtls_mpi_read_binary(&b, right, 32U);
    if (result == 0 &&
        (mbedtls_mpi_cmp_int(&a, 0) <= 0 ||
         mbedtls_mpi_cmp_mpi(&a, &group.N) >= 0))
        result = -1;
    if (result == 0 &&
        (mbedtls_mpi_cmp_int(&b, 0) <= 0 ||
         mbedtls_mpi_cmp_mpi(&b, &group.N) >= 0))
        result = -1;
    if (result == 0) result = mbedtls_mpi_add_mpi(&sum, &a, &b);
    if (result == 0) result = mbedtls_mpi_mod_mpi(&sum, &sum, &group.N);
    if (result == 0 && mbedtls_mpi_cmp_int(&sum, 0) == 0) result = -1;
    if (result == 0) result = mbedtls_mpi_write_binary(&sum, output, 32U);

    mbedtls_mpi_free(&sum);
    mbedtls_mpi_free(&b);
    mbedtls_mpi_free(&a);
    mbedtls_ecp_group_free(&group);
    return result == 0;
}

#else

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>

bool bc2_secp256k1_public(const uint8_t k[32],uint8_t o[33]){if(!k||!o)return false;EC_GROUP*g=EC_GROUP_new_by_curve_name(NID_secp256k1);BN_CTX*c=BN_CTX_new();BIGNUM*b=BN_bin2bn(k,32,NULL);EC_POINT*q=g?EC_POINT_new(g):NULL;bool ok=g&&c&&b&&q&&!BN_is_zero(b)&&EC_POINT_mul(g,q,b,NULL,NULL,c)==1&&EC_POINT_point2oct(g,q,POINT_CONVERSION_COMPRESSED,o,33,c)==33U;EC_POINT_free(q);BN_clear_free(b);BN_CTX_free(c);EC_GROUP_free(g);return ok;}

static bool add_private_scalars(const uint8_t left[32], const uint8_t right[32],
                                uint8_t output[32]) {
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *a = BN_bin2bn(left, 32, NULL);
    BIGNUM *b = BN_bin2bn(right, 32, NULL);
    BIGNUM *order = BN_new();
    bool ok = group && ctx && a && b && order &&
              EC_GROUP_get_order(group, order, ctx) == 1 &&
              !BN_is_zero(a) && BN_cmp(a, order) < 0 &&
              !BN_is_zero(b) && BN_cmp(b, order) < 0 &&
              BN_mod_add(a, a, b, order, ctx) == 1 &&
              !BN_is_zero(a) &&
              BN_bn2binpad(a, output, 32) == 32;
    BN_clear_free(a);
    BN_clear_free(b);
    BN_free(order);
    BN_CTX_free(ctx);
    EC_GROUP_free(group);
    return ok;
}

#endif

static uint32_t fp(const bc2_xprv *x) {
    uint8_t public_key[33], hash[20];
    if (!bc2_secp256k1_public(x->key, public_key) ||
        !bc2_hash160(public_key, sizeof(public_key), hash))
        return 0U;
    return ((uint32_t)hash[0] << 24) | ((uint32_t)hash[1] << 16) |
           ((uint32_t)hash[2] << 8) | hash[3];
}

bool bc2_bip32_master(const uint8_t *seed, size_t seed_size, bc2_xprv *output) {
    static const uint8_t key[] = "Bitcoin seed";
    uint8_t hash[64];
    if (!seed || !output ||
        !bc2_hmac_sha512(key, sizeof(key) - 1U, seed, seed_size, hash))
        return false;
    memset(output, 0, sizeof(*output));
    memcpy(output->key, hash, 32U);
    memcpy(output->chain, hash + 32U, 32U);
    memset(hash, 0, sizeof(hash));
    return true;
}

bool bc2_bip32_derive(const bc2_xprv *parent, uint32_t index, bc2_xprv *output) {
    uint8_t data[37];
    uint8_t hash[64];
    uint8_t public_key[33];
    uint8_t child_key[32];

    if (!parent || !output || parent->depth == 255U) return false;

    if ((index & 0x80000000U) != 0U) {
        data[0] = 0U;
        memcpy(data + 1U, parent->key, 32U);
    } else {
        if (!bc2_secp256k1_public(parent->key, public_key)) return false;
        memcpy(data, public_key, 33U);
    }
    be32(data + 33U, index);

    if (!bc2_hmac_sha512(parent->chain, 32U, data, sizeof(data), hash))
        return false;
    if (!add_private_scalars(hash, parent->key, child_key)) {
        memset(hash, 0, sizeof(hash));
        return false;
    }

    memset(output, 0, sizeof(*output));
    memcpy(output->key, child_key, 32U);
    memcpy(output->chain, hash + 32U, 32U);
    output->depth = (uint8_t)(parent->depth + 1U);
    output->child = index;
    output->parent_fingerprint = fp(parent);

    memset(hash, 0, sizeof(hash));
    memset(child_key, 0, sizeof(child_key));
    memset(public_key, 0, sizeof(public_key));
    return true;
}

bool bc2_bip32_derive_path(const bc2_xprv *master, const char *path, bc2_xprv *output) {
    if (!master || !path || !output || path[0] != 'm') return false;
    bc2_xprv current = *master;
    if (path[1] == 0) { *output = current; return true; }
    if (path[1] != '/') return false;
    char buffer[256];
    if (strlen(path) >= sizeof(buffer)) return false;
    if (!bc2_copy_string(buffer, sizeof(buffer), path + 2)) return false;

    char *save = NULL;
    for (char *token = bc2_strtok(buffer, "/", &save);
         token;
         token = bc2_strtok(NULL, "/", &save)) {
        size_t length = strlen(token);
        bool hardened = length > 0U && token[length - 1U] == '\'';
        if (hardened) token[--length] = 0;
        if (length == 0U) return false;
        char *end = NULL;
        unsigned long value = strtoul(token, &end, 10);
        if (!end || *end || value > 0x7fffffffUL) return false;
        uint32_t child = (uint32_t)value | (hardened ? 0x80000000U : 0U);
        bc2_xprv next;
        if (!bc2_bip32_derive(&current, child, &next)) return false;
        current = next;
    }
    *output = current;
    return true;
}

static bool ser(const bc2_xprv *x, uint32_t version, bool public_version,
                char *output, size_t output_size) {
    uint8_t bytes[78];
    be32(bytes, version);
    bytes[4] = x->depth;
    be32(bytes + 5, x->parent_fingerprint);
    be32(bytes + 9, x->child);
    memcpy(bytes + 13, x->chain, 32);
    if (public_version) {
        if (!bc2_secp256k1_public(x->key, bytes + 45)) return false;
    } else {
        bytes[45] = 0;
        memcpy(bytes + 46, x->key, 32);
    }
    return bc2_base58check_encode(bytes, 78, output, output_size);
}

bool bc2_xprv_serialize(const bc2_xprv*x,uint32_t v,char*o,size_t z){return x&&ser(x,v,false,o,z);}
bool bc2_xpub_serialize(const bc2_xprv*x,uint32_t v,char*o,size_t z){return x&&ser(x,v,true,o,z);}
