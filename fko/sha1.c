/* $Id$
 *****************************************************************************
 *
 * File:    sha1.c
 *
 * Purpose: Implementation of the SHA1 message-digest algorithm for
 *          libfwknop.
 *
 * NIST Secure Hash Algorithm
 *   Heavily modified by Uwe Hollerbach <uh@alumni.caltech edu>
 *   from Peter C. Gutmann's implementation as found in
 *   Applied Cryptography by Bruce Schneier
 *   Further modifications to include the "UNRAVEL" stuff, below
 *
 * This code is in the public domain
 *
 *****************************************************************************
*/
#include "sha.h"

/* SHA f()-functions */
#define f1(x,y,z)    ((x & y) | (~x & z))
#define f2(x,y,z)    (x ^ y ^ z)
#define f3(x,y,z)    ((x & y) | (x & z) | (y & z))
#define f4(x,y,z)    (x ^ y ^ z)

/* SHA constants */
#define CONST1        0x5a827999L
#define CONST2        0x6ed9eba1L
#define CONST3        0x8f1bbcdcL
#define CONST4        0xca62c1d6L

/* truncate to 32 bits -- should be a null op on 32-bit machines */
#define T32(x)    ((x) & 0xffffffffL)

/* 32-bit rotate */
#define R32(x,n)    T32(((x << n) | (x >> (32 - n))))

/* the generic case, for when the overall rotation is not unraveled */
#define FG(n)    \
    T = T32(R32(A,5) + f##n(B,C,D) + E + *WP++ + CONST##n);    \
    E = D; D = C; C = R32(B,30); B = A; A = T

/* specific cases, for when the overall rotation is unraveled */
#define FA(n)    \
    T = T32(R32(A,5) + f##n(B,C,D) + E + *WP++ + CONST##n); B = R32(B,30)

#define FB(n)    \
    E = T32(R32(T,5) + f##n(A,B,C) + D + *WP++ + CONST##n); A = R32(A,30)

#define FC(n)    \
    D = T32(R32(E,5) + f##n(T,A,B) + C + *WP++ + CONST##n); T = R32(T,30)

#define FD(n)    \
    C = T32(R32(D,5) + f##n(E,T,A) + B + *WP++ + CONST##n); E = R32(E,30)

#define FE(n)    \
    B = T32(R32(C,5) + f##n(D,E,T) + A + *WP++ + CONST##n); D = R32(D,30)

#define FT(n)    \
    A = T32(R32(B,5) + f##n(C,D,E) + T + *WP++ + CONST##n); C = R32(C,30)


void
sha1_transform(SHA_INFO *sha_info)
{
    int i;
    uint8_t *dp;
    uint32_t T, A, B, C, D, E, W[80], *WP;

    dp = sha_info->data;

#undef SWAP_DONE

#if BYTEORDER == 1234
#define SWAP_DONE
    for (i = 0; i < 16; ++i) {
        T = *((uint32_t *) dp);
        dp += 4;
        W[i] = 
            ((T << 24) & 0xff000000) |
            ((T <<  8) & 0x00ff0000) |
            ((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
    }
#endif

#if BYTEORDER == 4321
#define SWAP_DONE
    for (i = 0; i < 16; ++i) {
        T = *((uint32_t *) dp);
        dp += 4;
        W[i] = TRUNC32(T);
    }
#endif

#if BYTEORDER == 12345678
#define SWAP_DONE
    for (i = 0; i < 16; i += 2) {
        T = *((uint32_t *) dp);
        dp += 8;
        W[i] =  ((T << 24) & 0xff000000) | ((T <<  8) & 0x00ff0000) |
            ((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
        T >>= 32;
        W[i+1] = ((T << 24) & 0xff000000) | ((T <<  8) & 0x00ff0000) |
            ((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
    }
#endif

#if BYTEORDER == 87654321
#define SWAP_DONE
    for (i = 0; i < 16; i += 2) {
        T = *((uint32_t *) dp);
        dp += 8;
        W[i] = TRUNC32(T >> 32);
        W[i+1] = TRUNC32(T);
    }
#endif

#ifndef SWAP_DONE
#define SWAP_DONE
    for (i = 0; i < 16; ++i) {
        T = *((uint32_t *) dp);
        dp += 4;
        W[i] = TRUNC32(T);
    }
  #ifndef WIN32
    #warning Undetermined or unsupported Byte Order... We will try LITTLE_ENDIAN
  #endif
#endif /* SWAP_DONE */

    for (i = 16; i < 80; ++i) {
    W[i] = W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
    W[i] = R32(W[i], 1);
    }
    A = sha_info->digest[0];
    B = sha_info->digest[1];
    C = sha_info->digest[2];
    D = sha_info->digest[3];
    E = sha_info->digest[4];
    WP = W;
#ifdef UNRAVEL
    FA(1); FB(1); FC(1); FD(1); FE(1); FT(1); FA(1); FB(1); FC(1); FD(1);
    FE(1); FT(1); FA(1); FB(1); FC(1); FD(1); FE(1); FT(1); FA(1); FB(1);
    FC(2); FD(2); FE(2); FT(2); FA(2); FB(2); FC(2); FD(2); FE(2); FT(2);
    FA(2); FB(2); FC(2); FD(2); FE(2); FT(2); FA(2); FB(2); FC(2); FD(2);
    FE(3); FT(3); FA(3); FB(3); FC(3); FD(3); FE(3); FT(3); FA(3); FB(3);
    FC(3); FD(3); FE(3); FT(3); FA(3); FB(3); FC(3); FD(3); FE(3); FT(3);
    FA(4); FB(4); FC(4); FD(4); FE(4); FT(4); FA(4); FB(4); FC(4); FD(4);
    FE(4); FT(4); FA(4); FB(4); FC(4); FD(4); FE(4); FT(4); FA(4); FB(4);
    sha_info->digest[0] = T32(sha_info->digest[0] + E);
    sha_info->digest[1] = T32(sha_info->digest[1] + T);
    sha_info->digest[2] = T32(sha_info->digest[2] + A);
    sha_info->digest[3] = T32(sha_info->digest[3] + B);
    sha_info->digest[4] = T32(sha_info->digest[4] + C);
#else /* !UNRAVEL */
#ifdef UNROLL_LOOPS
    FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1);
    FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1); FG(1);
    FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2);
    FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2); FG(2);
    FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3);
    FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3); FG(3);
    FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4);
    FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4); FG(4);
#else /* !UNROLL_LOOPS */
    for (i =  0; i < 20; ++i) { FG(1); }
    for (i = 20; i < 40; ++i) { FG(2); }
    for (i = 40; i < 60; ++i) { FG(3); }
    for (i = 60; i < 80; ++i) { FG(4); }
#endif /* !UNROLL_LOOPS */
    sha_info->digest[0] = T32(sha_info->digest[0] + A);
    sha_info->digest[1] = T32(sha_info->digest[1] + B);
    sha_info->digest[2] = T32(sha_info->digest[2] + C);
    sha_info->digest[3] = T32(sha_info->digest[3] + D);
    sha_info->digest[4] = T32(sha_info->digest[4] + E);
#endif /* !UNRAVEL */
}

/* initialize the SHA digest */

void
sha1_init(SHA_INFO *sha_info)
{
    sha_info->digest[0] = 0x67452301L;
    sha_info->digest[1] = 0xefcdab89L;
    sha_info->digest[2] = 0x98badcfeL;
    sha_info->digest[3] = 0x10325476L;
    sha_info->digest[4] = 0xc3d2e1f0L;
    sha_info->count_lo = 0L;
    sha_info->count_hi = 0L;
    sha_info->local = 0;
}

/* update the SHA digest */

void
sha1_update(SHA_INFO *sha_info, uint8_t *buffer, int count)
{
    int i;
    uint32_t clo;

    clo = T32(sha_info->count_lo + ((uint32_t) count << 3));
    if (clo < sha_info->count_lo) {
    ++sha_info->count_hi;
    }
    sha_info->count_lo = clo;
    sha_info->count_hi += (uint32_t) count >> 29;
    if (sha_info->local) {
    i = SHA_BLOCKSIZE - sha_info->local;
    if (i > count) {
        i = count;
    }
    memcpy(((uint8_t *) sha_info->data) + sha_info->local, buffer, i);
    count -= i;
    buffer += i;
    sha_info->local += i;
    if (sha_info->local == SHA_BLOCKSIZE) {
        sha1_transform(sha_info);
    } else {
        return;
    }
    }
    while (count >= SHA_BLOCKSIZE) {
    memcpy(sha_info->data, buffer, SHA_BLOCKSIZE);
    buffer += SHA_BLOCKSIZE;
    count -= SHA_BLOCKSIZE;
    sha1_transform(sha_info);
    }
    memcpy(sha_info->data, buffer, count);
    sha_info->local = count;
}


void
sha1_transform_and_copy(unsigned char digest[20], SHA_INFO *sha_info)
{
    sha1_transform(sha_info);
    digest[ 0] = (unsigned char) ((sha_info->digest[0] >> 24) & 0xff);
    digest[ 1] = (unsigned char) ((sha_info->digest[0] >> 16) & 0xff);
    digest[ 2] = (unsigned char) ((sha_info->digest[0] >>  8) & 0xff);
    digest[ 3] = (unsigned char) ((sha_info->digest[0]      ) & 0xff);
    digest[ 4] = (unsigned char) ((sha_info->digest[1] >> 24) & 0xff);
    digest[ 5] = (unsigned char) ((sha_info->digest[1] >> 16) & 0xff);
    digest[ 6] = (unsigned char) ((sha_info->digest[1] >>  8) & 0xff);
    digest[ 7] = (unsigned char) ((sha_info->digest[1]      ) & 0xff);
    digest[ 8] = (unsigned char) ((sha_info->digest[2] >> 24) & 0xff);
    digest[ 9] = (unsigned char) ((sha_info->digest[2] >> 16) & 0xff);
    digest[10] = (unsigned char) ((sha_info->digest[2] >>  8) & 0xff);
    digest[11] = (unsigned char) ((sha_info->digest[2]      ) & 0xff);
    digest[12] = (unsigned char) ((sha_info->digest[3] >> 24) & 0xff);
    digest[13] = (unsigned char) ((sha_info->digest[3] >> 16) & 0xff);
    digest[14] = (unsigned char) ((sha_info->digest[3] >>  8) & 0xff);
    digest[15] = (unsigned char) ((sha_info->digest[3]      ) & 0xff);
    digest[16] = (unsigned char) ((sha_info->digest[4] >> 24) & 0xff);
    digest[17] = (unsigned char) ((sha_info->digest[4] >> 16) & 0xff);
    digest[18] = (unsigned char) ((sha_info->digest[4] >>  8) & 0xff);
    digest[19] = (unsigned char) ((sha_info->digest[4]      ) & 0xff);
}

/* finish computing the SHA digest */
void
sha1_final(uint8_t digest[20], SHA_INFO *sha_info)
{
    int count;
    uint32_t lo_bit_count, hi_bit_count;

    lo_bit_count = sha_info->count_lo;
    hi_bit_count = sha_info->count_hi;
    count = (int) ((lo_bit_count >> 3) & 0x3f);
    ((uint8_t *) sha_info->data)[count++] = 0x80;
    if (count > SHA_BLOCKSIZE - 8) {
    memset(((uint8_t *) sha_info->data) + count, 0, SHA_BLOCKSIZE - count);
    sha1_transform(sha_info);
    memset((uint8_t *) sha_info->data, 0, SHA_BLOCKSIZE - 8);
    } else {
    memset(((uint8_t *) sha_info->data) + count, 0,
        SHA_BLOCKSIZE - 8 - count);
    }
    sha_info->data[56] = (uint8_t)((hi_bit_count >> 24) & 0xff);
    sha_info->data[57] = (uint8_t)((hi_bit_count >> 16) & 0xff);
    sha_info->data[58] = (uint8_t)((hi_bit_count >>  8) & 0xff);
    sha_info->data[59] = (uint8_t)((hi_bit_count >>  0) & 0xff);
    sha_info->data[60] = (uint8_t)((lo_bit_count >> 24) & 0xff);
    sha_info->data[61] = (uint8_t)((lo_bit_count >> 16) & 0xff);
    sha_info->data[62] = (uint8_t)((lo_bit_count >>  8) & 0xff);
    sha_info->data[63] = (uint8_t)((lo_bit_count >>  0) & 0xff);
    sha1_transform_and_copy(digest, sha_info);
}

/***EOF***/
