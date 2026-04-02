#include <sshash.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#define TEST_SIZE (1ull << 30)

static double now_sec(void){
#if defined(_WIN32)
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

static inline void print_hex(const uint8_t* hex, size_t bytes){
    for(size_t i = 0; i < bytes; ++i)
        printf("%02X", hex[i]);
}

int main(void){
    uint8_t* test_buf = (uint8_t*)malloc(TEST_SIZE);
    if(!test_buf){
        printf("Allocation failed\n");
        return 1;
    }

    for(size_t i = 0; i < TEST_SIZE; ++i)
        test_buf[i] = 0;
        
    uint32_t state[8] = {0};
    uint32_t hash[4] = {0};
    uint32_t ctr = 0;
    double start = now_sec();
    ctr = sshasha128(state, ctr, test_buf, TEST_SIZE);
    sshashs128(state, hash);
    double end = now_sec();
    double elapsed = end - start;
    printf("sshash128:\n");
    printf("    Time(%.9f s)\n", elapsed);
    printf("    Speed(%.2f GB/s)\n", (double)TEST_SIZE / elapsed / 1e9);
    printf("    Hash(0x");
    print_hex((const uint8_t*)hash, sizeof(hash));
    printf(")\n\n");

    uint64_t state64[8] = {0};
    uint64_t hash64[4] = {0};
    uint64_t ctr64 = 0;
    start = now_sec();
    ctr = sshasha256(state64, ctr64, test_buf, TEST_SIZE);
    sshashs256(state64, hash64);
    end = now_sec();
    elapsed = end - start;
    printf("sshash256:\n");
    printf("    Time(%.9f s)\n", elapsed);
    printf("    Speed(%.2f GB/s)\n", (double)TEST_SIZE / elapsed / 1e9);
    printf("    Hash(0x");
    print_hex((const uint8_t*)hash64, sizeof(hash64));
    printf(")\n");
    free(test_buf);
    return 0;
}