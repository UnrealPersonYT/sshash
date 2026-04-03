#include <sshash.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define SAMPLES 25000000ULL
#define BUCKETS 65536ULL

// =======================================================
//              MATH CORE
// =======================================================

static double normal_tail(double z) {
    return 0.5 * erfc(z / sqrt(2.0));
}

static double z_from_p(double p) {
    return sqrt(-2.0 * log(p));
}

static double compute_alpha(uint64_t n) {
    return 1.0 / sqrt((double)n);
}

static double compute_z_threshold(uint64_t n) {
    return z_from_p(compute_alpha(n));
}

// =======================================================
//                  PRNG
// =======================================================

static uint64_t prng_state = 0x123456789ABCDEF0ULL;

static inline uint64_t prng64(void) {
    prng_state = prng_state * 6364136223846793005ULL + 1;
    return prng_state;
}

static inline void fill_random(uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i += 8) {
        uint64_t x = prng64();
        memcpy(data + i, &x, 8);
    }
}

// =======================================================
//              DISTRIBUTION CORE
// =======================================================

static void distribution_test(const char *name,
                              uint32_t buckets[BUCKETS]) {

    double expected = (double)SAMPLES / (double)BUCKETS;
    double chi2 = 0.0;

    for (uint64_t i = 0; i < BUCKETS; i++) {
        double d = buckets[i] - expected;
        chi2 += (d * d) / expected;
    }

    double dof = BUCKETS - 1;
    double z = (chi2 - dof) / sqrt(2.0 * dof);

    double alpha = compute_alpha(SAMPLES);
    double z_thr = compute_z_threshold(SAMPLES);

    printf("\n=== %s Distribution ===\n", name);
    printf("Chi2 Z: %.3f\n", z);
    printf("Alpha: %.10f\n", alpha);
    printf("Z threshold: %.3f\n", z_thr);

    printf("Status: %s\n",
           fabs(z) < z_thr ? "PASS" : "FAIL");
}

// =======================================================
//              256-bit DISTRIBUTION
// =======================================================

void test_distribution_256(void) {

    uint64_t state[8], hash[4];
    uint32_t buckets[BUCKETS];
    memset(buckets, 0, sizeof(buckets));

    printf("\n=== 256-bit Distribution ===\n");

    for (uint64_t i = 0; i < SAMPLES; i++) {

        uint8_t data[32];
        fill_random(data, sizeof(data));

        sshasha256(state, i, data, 32);
        sshashs256(state, hash);

        buckets[(uint16_t)(hash[0])]++;
    }

    distribution_test("256-bit", buckets);
}

// =======================================================
//              128-bit DISTRIBUTION (FIXED)
// =======================================================

void test_distribution_128(void) {

    uint32_t state[8], hash[4];
    uint32_t buckets[BUCKETS];
    memset(buckets, 0, sizeof(buckets));

    printf("\n=== 128-bit Distribution ===\n");

    for (uint64_t i = 0; i < SAMPLES; i++) {

        uint8_t data[32];
        fill_random(data, sizeof(data));

        sshasha128(state, i, data, 32);
        sshashs128(state, hash);

        buckets[(uint16_t)(hash[0])]++;
    }

    distribution_test("128-bit", buckets);
}

// =======================================================
//              AVALANCHE TEST
// =======================================================

void avalanche_test(int bits, int is128) {

    printf("\n=== %d-bit Avalanche ===\n", bits);

    const double expected = bits * 0.5;
    const double variance = bits * 0.25;

    double sum = 0.0;
    double sum_sq = 0.0;

    for (uint64_t i = 0; i < SAMPLES; i++) {

        uint8_t d1[64], d2[64];

        fill_random(d1, sizeof(d1));
        memcpy(d2, d1, sizeof(d1));

        d2[prng64() % 64] ^= (uint8_t)(1u << (prng64() % 8));

        uint32_t diff = 0;

        if (!is128) {

            uint64_t s1[8]={0}, s2[8]={0}, h1[4], h2[4];

            sshasha256(s1, 0, d1, 64);
            sshashs256(s1, h1);

            sshasha256(s2, 0, d2, 64);
            sshashs256(s2, h2);

            for (int j = 0; j < 4; j++)
                diff += __builtin_popcountll(h1[j] ^ h2[j]);

        } else {

            uint32_t s1[8]={0}, s2[8]={0}, h1[4], h2[4];

            sshasha128(s1, 0, d1, 64);
            sshashs128(s1, h1);

            sshasha128(s2, 0, d2, 64);
            sshashs128(s2, h2);

            for (int j = 0; j < 4; j++)
                diff += __builtin_popcount(h1[j] ^ h2[j]);
        }

        sum += diff;
        sum_sq += (double)diff * diff;
    }

    double mean = sum / SAMPLES;
    double var = (sum_sq / SAMPLES) - (mean * mean);
    double stddev = sqrt(var);

    double z = (mean - expected) / sqrt(variance / SAMPLES);

    printf("Mean: %.6f (expected %.2f)\n", mean, expected);
    printf("Stddev(obs): %.6f\n", stddev);
    printf("Z-score: %.4f\n", z);

    double alpha = compute_alpha(SAMPLES);
    double z_thr = compute_z_threshold(SAMPLES);

    printf("Alpha: %.10f\n", alpha);
    printf("Z threshold: %.3f\n", z_thr);

    printf("Status: %s\n",
           fabs(z) < z_thr ? "PASS" : "FAIL");
}

// =======================================================
//              PATTERN TEST
// =======================================================

void pattern_test(int bits, int is128) {

    printf("\n=== %d-bit Pattern Test ===\n", bits);

    const double mean = bits * 0.5;
    const double sigma = sqrt(bits * 0.25);

    double alpha = compute_alpha(SAMPLES);

    uint64_t weak = 0;

    uint8_t patterns[] = {0x00, 0xFF, 0xAA, 0x55};

    for (size_t p = 0; p < 4; p++) {

        for (uint64_t i = 0; i < SAMPLES / 4; i++) {

            uint8_t d1[32], d2[32];
            memset(d1, patterns[p], sizeof(d1));
            memset(d2, patterns[p] ^ 0xFF, sizeof(d2));

            uint32_t diff = 0;

            if (!is128) {

                uint64_t s1[8]={0}, s2[8]={0}, h1[4], h2[4];

                sshasha256(s1, 0, d1, 32);
                sshashs256(s1, h1);

                sshasha256(s2, 0, d2, 32);
                sshashs256(s2, h2);

                for (int j = 0; j < 4; j++)
                    diff += __builtin_popcountll(h1[j] ^ h2[j]);

            } else {

                uint32_t s1[8]={0}, s2[8]={0}, h1[4], h2[4];

                sshasha128(s1, 0, d1, 32);
                sshashs128(s1, h1);

                sshasha128(s2, 0, d2, 32);
                sshashs128(s2, h2);

                for (int j = 0; j < 4; j++)
                    diff += __builtin_popcount(h1[j] ^ h2[j]);
            }

            double z = fabs((diff - mean) / sigma);
            double p = normal_tail(z) * 2.0;

            if (p < alpha)
                weak++;
        }
    }

    double observed = (double)weak / SAMPLES;

    printf("Weak rate: %.8f%%\n", observed * 100.0);
    printf("Alpha: %.10f\n", alpha);
    printf("Status: %s\n",
           observed < alpha ? "PASS" : "FAIL");
}

// =======================================================
//                      MAIN
// =======================================================

int main(void) {

    srand((unsigned)time(NULL));

    printf("\n====================================\n");
    printf(" SELF-SCALING STATISTICAL SUITE\n");
    printf(" N = %llu\n", (unsigned long long)SAMPLES);
    printf("====================================\n");

    test_distribution_256();
    test_distribution_128();

    avalanche_test(256, 0);
    avalanche_test(128, 1);

    pattern_test(256, 0);
    pattern_test(128, 1);

    printf("\nDONE\n");
    return 0;
}