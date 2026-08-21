#include "bc2_sign.h"
#include "bc2_crypto.h"

#include <string.h>

#ifdef ESP_PLATFORM

#include "mbedtls/bignum.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/ecp.h"
#include "esp_random.h"

static int bc2_sign_rng(void *context, unsigned char *output, size_t length)
{
    (void)context;

    if (output == NULL) {
        return -1;
    }

    esp_fill_random(output, length);
    return 0;
}

static bool bc2_der_write_integer(
    const mbedtls_mpi *value,
    uint8_t *output,
    size_t capacity,
    size_t *written)
{
    uint8_t raw[32];
    size_t first = 0U;
    size_t size;
    bool prefix_zero;
    size_t total;

    if (value == NULL || output == NULL || written == NULL) {
        return false;
    }

    memset(raw, 0, sizeof(raw));

    if (mbedtls_mpi_write_binary(value, raw, sizeof(raw)) != 0) {
        return false;
    }

    while (first < sizeof(raw) - 1U && raw[first] == 0U) {
        ++first;
    }

    size = sizeof(raw) - first;
    prefix_zero = (raw[first] & 0x80U) != 0U;
    total = 2U + size + (prefix_zero ? 1U : 0U);

    if (capacity < total || size > 0x7fU) {
        return false;
    }

    output[0] = 0x02U;
    output[1] = (uint8_t)(size + (prefix_zero ? 1U : 0U));

    size_t offset = 2U;

    if (prefix_zero) {
        output[offset++] = 0x00U;
    }

    memcpy(output + offset, raw + first, size);
    *written = total;

    memset(raw, 0, sizeof(raw));
    return true;
}

static bool bc2_der_encode_signature(
    const mbedtls_mpi *r,
    const mbedtls_mpi *s,
    uint8_t *output,
    size_t capacity,
    size_t *output_length)
{
    uint8_t encoded_r[35];
    uint8_t encoded_s[35];
    size_t r_length = 0U;
    size_t s_length = 0U;
    size_t body_length;

    if (r == NULL || s == NULL || output == NULL || output_length == NULL) {
        return false;
    }

    *output_length = 0U;

    if (!bc2_der_write_integer(
            r,
            encoded_r,
            sizeof(encoded_r),
            &r_length) ||
        !bc2_der_write_integer(
            s,
            encoded_s,
            sizeof(encoded_s),
            &s_length)) {
        return false;
    }

    body_length = r_length + s_length;

    if (body_length > 0x7fU || capacity < 2U + body_length) {
        return false;
    }

    output[0] = 0x30U;
    output[1] = (uint8_t)body_length;

    memcpy(output + 2U, encoded_r, r_length);
    memcpy(output + 2U + r_length, encoded_s, s_length);

    *output_length = 2U + body_length;

    memset(encoded_r, 0, sizeof(encoded_r));
    memset(encoded_s, 0, sizeof(encoded_s));
    return true;
}

static bool bc2_der_read_integer(
    const uint8_t *input,
    size_t input_length,
    size_t *offset,
    mbedtls_mpi *value)
{
    size_t length;
    const uint8_t *data;

    if (input == NULL || offset == NULL || value == NULL ||
        *offset + 2U > input_length ||
        input[*offset] != 0x02U) {
        return false;
    }

    ++(*offset);
    length = input[(*offset)++];

    if (length == 0U || length > 33U ||
        *offset + length > input_length) {
        return false;
    }

    data = input + *offset;

    if (length == 33U) {
        if (data[0] != 0x00U) {
            return false;
        }

        ++data;
        --length;
    }

    if (mbedtls_mpi_read_binary(value, data, length) != 0) {
        return false;
    }

    *offset += input[*offset - 1U];
    return true;
}

bool bc2_ecdsa_sign_der(
    const uint8_t private_key[32],
    const uint8_t hash[32],
    uint8_t *output,
    size_t capacity,
    size_t *output_length)
{
    mbedtls_ecp_group group;
    mbedtls_mpi d;
    mbedtls_mpi r;
    mbedtls_mpi s;
    mbedtls_mpi half_order;
    int result;
    bool ok = false;

    if (private_key == NULL || hash == NULL ||
        output == NULL || output_length == NULL) {
        return false;
    }

    *output_length = 0U;

    mbedtls_ecp_group_init(&group);
    mbedtls_mpi_init(&d);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    mbedtls_mpi_init(&half_order);

    result = mbedtls_ecp_group_load(
        &group,
        MBEDTLS_ECP_DP_SECP256K1);

    if (result == 0) {
        result = mbedtls_mpi_read_binary(
            &d,
            private_key,
            32U);
    }

    if (result == 0 &&
        (mbedtls_mpi_cmp_int(&d, 1) < 0 ||
         mbedtls_mpi_cmp_mpi(&d, &group.N) >= 0)) {
        result = -1;
    }

    if (result == 0) {
        result = mbedtls_ecdsa_sign(
            &group,
            &r,
            &s,
            &d,
            hash,
            32U,
            bc2_sign_rng,
            NULL);
    }

    /*
     * Bitcoin standardness requires low-S signatures.
     * Normalize S to min(S, curve_order - S).
     */
    if (result == 0) {
        result = mbedtls_mpi_copy(
            &half_order,
            &group.N);
    }

    if (result == 0) {
        result = mbedtls_mpi_shift_r(
            &half_order,
            1U);
    }

    if (result == 0 &&
        mbedtls_mpi_cmp_mpi(&s, &half_order) > 0) {
        result = mbedtls_mpi_sub_mpi(
            &s,
            &group.N,
            &s);
    }

    if (result == 0) {
        ok = bc2_der_encode_signature(
            &r,
            &s,
            output,
            capacity,
            output_length);
    }

    mbedtls_mpi_free(&half_order);
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&d);
    mbedtls_ecp_group_free(&group);

    return ok;
}

bool bc2_ecdsa_verify_der(
    const uint8_t public_key[33],
    const uint8_t hash[32],
    const uint8_t *signature,
    size_t signature_length)
{
    mbedtls_ecp_group group;
    mbedtls_ecp_point point;
    mbedtls_mpi r;
    mbedtls_mpi s;
    size_t offset = 0U;
    size_t sequence_length;
    int result;
    bool ok = false;

    if (public_key == NULL || hash == NULL ||
        signature == NULL || signature_length < 8U) {
        return false;
    }

    if (signature[0] != 0x30U) {
        return false;
    }

    sequence_length = signature[1];

    if (sequence_length + 2U != signature_length) {
        return false;
    }

    offset = 2U;

    mbedtls_ecp_group_init(&group);
    mbedtls_ecp_point_init(&point);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    result = mbedtls_ecp_group_load(
        &group,
        MBEDTLS_ECP_DP_SECP256K1);

    if (result == 0) {
        result = mbedtls_ecp_point_read_binary(
            &group,
            &point,
            public_key,
            33U);
    }

    if (result == 0 &&
        (!bc2_der_read_integer(
             signature,
             signature_length,
             &offset,
             &r) ||
         !bc2_der_read_integer(
             signature,
             signature_length,
             &offset,
             &s) ||
         offset != signature_length)) {
        result = -1;
    }

    if (result == 0) {
        result = mbedtls_ecdsa_verify(
            &group,
            hash,
            32U,
            &point,
            &r,
            &s);
    }

    ok = result == 0;

    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    mbedtls_ecp_point_free(&point);
    mbedtls_ecp_group_free(&group);

    return ok;
}

#else

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

static EC_KEY *key_priv(const uint8_t private_key[32])
{
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
    BIGNUM *scalar = BN_bin2bn(private_key, 32, NULL);
    const EC_GROUP *group = key ? EC_KEY_get0_group(key) : NULL;
    EC_POINT *point = group ? EC_POINT_new(group) : NULL;

    if (key == NULL ||
        scalar == NULL ||
        point == NULL ||
        EC_KEY_set_private_key(key, scalar) != 1 ||
        EC_POINT_mul(group, point, scalar, NULL, NULL, NULL) != 1 ||
        EC_KEY_set_public_key(key, point) != 1) {
        EC_KEY_free(key);
        key = NULL;
    }

    EC_POINT_free(point);
    BN_clear_free(scalar);
    return key;
}

bool bc2_ecdsa_sign_der(
    const uint8_t private_key[32],
    const uint8_t hash[32],
    uint8_t *output,
    size_t capacity,
    size_t *output_length)
{
    EC_KEY *key;
    ECDSA_SIG *signature;
    const BIGNUM *r;
    const BIGNUM *s;
    const EC_GROUP *group;
    BIGNUM *order;
    BIGNUM *half_order;
    BIGNUM *low_s;
    BN_CTX *context;
    bool ok;

    if (private_key == NULL || hash == NULL ||
        output == NULL || output_length == NULL) {
        return false;
    }

    *output_length = 0U;

    key = key_priv(private_key);
    if (key == NULL) {
        return false;
    }

    signature = ECDSA_do_sign(hash, 32, key);

    if (signature == NULL) {
        EC_KEY_free(key);
        return false;
    }

    ECDSA_SIG_get0(signature, &r, &s);
    group = EC_KEY_get0_group(key);

    order = BN_new();
    half_order = BN_new();
    low_s = BN_dup(s);
    context = BN_CTX_new();

    ok = order != NULL &&
         half_order != NULL &&
         low_s != NULL &&
         context != NULL &&
         EC_GROUP_get_order(group, order, context) == 1 &&
         BN_rshift1(half_order, order) == 1;

    if (ok && BN_cmp(low_s, half_order) > 0) {
        ok = BN_sub(low_s, order, low_s) == 1;
    }

    if (ok) {
        BIGNUM *copy_r = BN_dup(r);
        ECDSA_SIG *canonical = ECDSA_SIG_new();

        ok = copy_r != NULL &&
             canonical != NULL &&
             ECDSA_SIG_set0(canonical, copy_r, low_s) == 1;

        low_s = NULL;

        if (ok) {
            int length = i2d_ECDSA_SIG(canonical, NULL);

            if (length > 0 && (size_t)length <= capacity) {
                unsigned char *cursor = output;
                ok = i2d_ECDSA_SIG(canonical, &cursor) == length;
                *output_length = ok ? (size_t)length : 0U;
            } else {
                ok = false;
            }
        }

        ECDSA_SIG_free(canonical);
    }

    BN_free(low_s);
    BN_free(order);
    BN_free(half_order);
    BN_CTX_free(context);
    ECDSA_SIG_free(signature);
    EC_KEY_free(key);

    return ok;
}

bool bc2_ecdsa_verify_der(
    const uint8_t public_key[33],
    const uint8_t hash[32],
    const uint8_t *signature,
    size_t signature_length)
{
    EC_KEY *key;
    const EC_GROUP *group;
    EC_POINT *point;
    bool ok;

    if (public_key == NULL || hash == NULL || signature == NULL) {
        return false;
    }

    key = EC_KEY_new_by_curve_name(NID_secp256k1);
    group = key ? EC_KEY_get0_group(key) : NULL;
    point = group ? EC_POINT_new(group) : NULL;

    ok = key != NULL &&
         point != NULL &&
         EC_POINT_oct2point(
             group,
             point,
             public_key,
             33U,
             NULL) == 1 &&
         EC_KEY_set_public_key(key, point) == 1 &&
         signature_length <= 2147483647U &&
         ECDSA_verify(
             0,
             hash,
             32,
             signature,
             (int)signature_length,
             key) == 1;

    EC_POINT_free(point);
    EC_KEY_free(key);

    return ok;
}

#endif

static int bc2_sign_put_u32(uint8_t *o,size_t c,size_t *x,uint32_t v){if(!o||!x||c-*x<4U)return 0;o[(*x)++]=(uint8_t)v;o[(*x)++]=(uint8_t)(v>>8U);o[(*x)++]=(uint8_t)(v>>16U);o[(*x)++]=(uint8_t)(v>>24U);return 1;}
static int bc2_sign_put_u64(uint8_t *o,size_t c,size_t *x,uint64_t v){if(!o||!x||c-*x<8U)return 0;for(unsigned i=0;i<8U;i++)o[(*x)++]=(uint8_t)(v>>(i*8U));return 1;}
static int bc2_sign_put_data(uint8_t*o,size_t c,size_t*x,const uint8_t*d,size_t n){if(!o||!x||(n&& !d)||n>c-*x)return 0;if(n)memcpy(o+*x,d,n);*x+=n;return 1;}
static int bc2_sign_put_script(uint8_t*o,size_t c,size_t*x,const uint8_t*s,size_t n){if(n>252U||*x>=c)return 0;o[(*x)++]=(uint8_t)n;return bc2_sign_put_data(o,c,x,s,n);}
bool bc2_p2wpkh_sighash_all_multi(
 const uint8_t hp[32],const uint8_t hs[32],const uint8_t ptx[32],uint32_t vout,
 uint64_t amount,const uint8_t pkh[20],uint32_t seq,
 const uint8_t*rs,size_t rn,uint64_t ra,const uint8_t*cs,size_t cn,uint64_t ca,
 uint32_t lock,uint8_t out[32]){
 uint8_t op[36],outs[512],pre[512],ho[32],script[26];size_t oo=0,po=0;
 if(!hp||!hs||!ptx||!pkh||!rs||rn==0||rn>252U||
    (ca>0U&&(!cs||cn==0U))||cn>252U||!out)return false;
 memcpy(op,ptx,32);op[32]=(uint8_t)vout;op[33]=(uint8_t)(vout>>8U);
 op[34]=(uint8_t)(vout>>16U);op[35]=(uint8_t)(vout>>24U);
 if(!bc2_sign_put_u64(outs,sizeof outs,&oo,ra)||
    !bc2_sign_put_script(outs,sizeof outs,&oo,rs,rn))return false;
 if(ca>0U&&(!bc2_sign_put_u64(outs,sizeof outs,&oo,ca)||
    !bc2_sign_put_script(outs,sizeof outs,&oo,cs,cn)))return false;
 if(!bc2_sha256d(outs,oo,ho))return false;
 script[0]=0x19;script[1]=0x76;script[2]=0xa9;script[3]=0x14;
 memcpy(script+4,pkh,20);script[24]=0x88;script[25]=0xac;
 if(!bc2_sign_put_u32(pre,sizeof pre,&po,2U)||
    !bc2_sign_put_data(pre,sizeof pre,&po,hp,32)||
    !bc2_sign_put_data(pre,sizeof pre,&po,hs,32)||
    !bc2_sign_put_data(pre,sizeof pre,&po,op,sizeof op)||
    !bc2_sign_put_data(pre,sizeof pre,&po,script,sizeof script)||
    !bc2_sign_put_u64(pre,sizeof pre,&po,amount)||
    !bc2_sign_put_u32(pre,sizeof pre,&po,seq)||
    !bc2_sign_put_data(pre,sizeof pre,&po,ho,32)||
    !bc2_sign_put_u32(pre,sizeof pre,&po,lock)||
    !bc2_sign_put_u32(pre,sizeof pre,&po,1U))return false;
 bool ok=bc2_sha256d(pre,po,out);
 memset(op,0,sizeof op);memset(outs,0,sizeof outs);memset(pre,0,sizeof pre);
 memset(ho,0,sizeof ho);memset(script,0,sizeof script);return ok;
}

bool bc2_p2wpkh_sighash_all_single(const uint8_t ptx[32],uint32_t vout,uint64_t amount,const uint8_t pkh[20],uint32_t seq,
 const uint8_t*rs,size_t rn,uint64_t ra,const uint8_t*cs,size_t cn,uint64_t ca,uint32_t lock,uint8_t out[32]){
 uint8_t op[36],sb[4],outs[512],pre[512],hp[32],hs[32],ho[32],sc[26];size_t oo=0,po=0;
 if(!ptx||!pkh||!rs||rn==0||rn>252U||(ca>0U&&(!cs||cn==0U))||cn>252U||!out)return false;
 memcpy(op,ptx,32);op[32]=(uint8_t)vout;op[33]=(uint8_t)(vout>>8U);op[34]=(uint8_t)(vout>>16U);op[35]=(uint8_t)(vout>>24U);
 sb[0]=(uint8_t)seq;sb[1]=(uint8_t)(seq>>8U);sb[2]=(uint8_t)(seq>>16U);sb[3]=(uint8_t)(seq>>24U);
 if(!bc2_sha256d(op,sizeof op,hp)||!bc2_sha256d(sb,sizeof sb,hs))return false;
 if(!bc2_sign_put_u64(outs,sizeof outs,&oo,ra)||!bc2_sign_put_script(outs,sizeof outs,&oo,rs,rn))return false;
 if(ca>0U&&(!bc2_sign_put_u64(outs,sizeof outs,&oo,ca)||!bc2_sign_put_script(outs,sizeof outs,&oo,cs,cn)))return false;
 if(!bc2_sha256d(outs,oo,ho))return false;
 sc[0]=0x19;sc[1]=0x76;sc[2]=0xa9;sc[3]=0x14;memcpy(sc+4,pkh,20);sc[24]=0x88;sc[25]=0xac;
 if(!bc2_sign_put_u32(pre,sizeof pre,&po,2U)||!bc2_sign_put_data(pre,sizeof pre,&po,hp,32)||
 !bc2_sign_put_data(pre,sizeof pre,&po,hs,32)||!bc2_sign_put_data(pre,sizeof pre,&po,op,sizeof op)||
 !bc2_sign_put_data(pre,sizeof pre,&po,sc,sizeof sc)||!bc2_sign_put_u64(pre,sizeof pre,&po,amount)||
 !bc2_sign_put_u32(pre,sizeof pre,&po,seq)||!bc2_sign_put_data(pre,sizeof pre,&po,ho,32)||
 !bc2_sign_put_u32(pre,sizeof pre,&po,lock)||!bc2_sign_put_u32(pre,sizeof pre,&po,1U))return false;
 bool ok=bc2_sha256d(pre,po,out);memset(op,0,sizeof op);memset(sb,0,sizeof sb);memset(outs,0,sizeof outs);memset(pre,0,sizeof pre);
 memset(hp,0,sizeof hp);memset(hs,0,sizeof hs);memset(ho,0,sizeof ho);memset(sc,0,sizeof sc);return ok;
}
