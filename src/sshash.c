#include <stdint.h>
#include <stddef.h>

#if !defined(B_ENDIAN) && !defined(_MSC_VER)
    #if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
        #define B_ENDIAN
    #endif
#endif  // !B_ENDIAN && !_MSC_VER

#if defined(_MSC_VER) && defined(B_ENDIAN)
    #include <intrin.h>
    #define __builtin_bswap64 _byteswap_uint64
    #define __builtin_bswap32 _byteswap_ulong
#endif  // _MSC_VER && B_ENDIAN

// How many rounds of permutation
#define SSHASH_SECURITY_MARGIN 12

static inline uint64_t sshashrol64(uint64_t x, size_t r){
    return (x << r) | (x >> (64 - r));
}
static inline void sshashmix64(uint64_t* restrict buf, size_t words, size_t rounds){
    for(size_t r = 0; r < rounds; ++r)
    for(size_t w = 0; w < words; ++w)
        if(!(w % 2))
            buf[w] = sshashrol64(buf[w] + buf[(w + (words - 1)) % words], !(w % 4) ? 29: 16);
        else
            buf[w] ^= buf[(w + (words - 1)) % words];
}
static inline uint32_t sshashrol32(uint32_t x, size_t r){
    return (x << r) | (x >> (32 - r));
}
static inline void sshashmix32(uint32_t* restrict buf, size_t words, size_t rounds){
    for(size_t r = 0; r < rounds; ++r)
    for(size_t w = 0; w < words; ++w)
        if(!(w % 2))
            buf[w] = sshashrol32(buf[w] + buf[(w + (words - 1)) % words], !(w % 4) ? 13: 8);
        else
            buf[w] ^= buf[(w + (words - 1)) % words];
}
uint64_t sshasha256(uint64_t state[8], uint64_t ctr, const void* restrict data, size_t datalen){
    size_t aligned = (datalen / (sizeof(uint64_t) * 4));
    for(size_t i = 0; i < aligned; ++i){
        state[0] ^= ctr++ + 1;
        for(size_t h = 0; h < 4; ++h)
        #ifdef B_ENDIAN
            state[h] ^= __builtin_bswap64(((const uint64_t*)data)[(i * 4) + h]);
        #else
            state[h] ^= ((const uint64_t*)data)[(i * 4) + h];
        #endif
        sshashmix64(state, 8, SSHASH_SECURITY_MARGIN);
    }
    size_t tail = datalen % (sizeof(uint64_t) * 4);
    if(tail){
        state[0] ^= ctr++;
        size_t words = tail / sizeof(uint64_t);
        for(size_t w = 0; w < words; ++w)
        #ifdef B_ENDIAN
            state[w] ^= __builtin_bswap64(((const uint64_t*)data)[(aligned * 4) + w]);
        #else
            state[w] ^= ((const uint64_t*)data)[(aligned * 4) + w];
        #endif
        size_t tail_bytes = (datalen - (aligned * (sizeof(uint64_t) * 4))) - (words * sizeof(uint64_t));
        if(tail_bytes){
            uint64_t dump = 0;
            for(size_t b = 0; b < tail_bytes; ++b)
                dump |= (uint64_t)((const uint8_t*)data)[(aligned * (sizeof(uint64_t) * 4)) + (words * sizeof(uint64_t)) + b] << (b * 8);
            dump |= 1ull << (tail_bytes * 8);
            state[words] ^= dump;
        }
        sshashmix64(state, 8, SSHASH_SECURITY_MARGIN);
    }
    return ctr;
}
void sshashs256(uint64_t state[8], uint64_t hash[4]){
    state[7] ^= 1;
    sshashmix64(state, 8, 8);
    for(size_t i = 0; i < 4; ++i)
        #ifdef B_ENDIAN
            hash[i] = __builtin_bswap64(state[i]);
        #else
            hash[i] = state[i];
        #endif
}
uint32_t sshasha128(uint32_t state[8], uint32_t ctr, const void* restrict data, size_t datalen){
    size_t aligned = (datalen / (sizeof(uint32_t) * 4));
    for(size_t i = 0; i < aligned; ++i){
        state[0] ^= ctr++ + 1;
        for(size_t h = 0; h < 4; ++h)
        #ifdef B_ENDIAN
            state[h] ^= __builtin_bswap32(((const uint32_t*)data)[(i * 4) + h]);
        #else
            state[h] ^= ((const uint32_t*)data)[(i * 4) + h];
        #endif
        sshashmix32(state, 8, SSHASH_SECURITY_MARGIN);
    }
    size_t tail = datalen % (sizeof(uint32_t) * 4);
    if(tail){
        state[0] ^= ctr++;
        size_t words = tail / sizeof(uint32_t);
        for(size_t w = 0; w < words; ++w)
        #ifdef B_ENDIAN
            state[w] ^= __builtin_bswap32(((const uint32_t*)data)[(aligned * 4) + w]);
        #else
            state[w] ^= ((const uint32_t*)data)[(aligned * 4) + w];
        #endif
        size_t tail_bytes = (datalen - (aligned * (sizeof(uint32_t) * 4))) - (words * sizeof(uint32_t));
        if(tail_bytes){
            uint32_t dump = 0;
            for(size_t b = 0; b < tail_bytes; ++b)
                dump |= (uint32_t)((const uint8_t*)data)[(aligned * (sizeof(uint32_t) * 4)) + (words * sizeof(uint32_t)) + b] << (b * 8);
            dump |= 1u << (tail_bytes * 8);
            state[words] ^= dump;
        }
        sshashmix32(state, 8, SSHASH_SECURITY_MARGIN);
    }
    return ctr;
}
void sshashs128(uint32_t state[8], uint32_t hash[4]){
    state[7] ^= 1;
    sshashmix32(state, 8, 8);
    for(size_t i = 0; i < 4; ++i)
        #ifdef B_ENDIAN
            hash[i] = __builtin_bswap32(state[i]);
        #else
            hash[i] = state[i];
        #endif
}