#include <sshash.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define SAMPLES 25000000ULL
#define BATCH 100000ULL

// Compute thresholds based on sample count
#define COLLISION_STDDEV_THRESHOLD (sqrt(SAMPLES / 65536.0) * 1.5)
#define AVALANCHE_THRESHOLD_256 (10.0)
#define AVALANCHE_THRESHOLD_128 (10.0)
#define WEAK_PERCENT_THRESHOLD (1.0)
#define BUCKETS 65536

// Tiny 64-bit LCG for reproducible pseudo-random bytes
static uint64_t prng_state = 0x123456789ABCDEF0ULL;
static inline uint64_t prng64(void) {
    prng_state = prng_state * 6364136223846793005ULL + 1;
    return prng_state;
}

static inline void fill_random_32(uint8_t data[32]) {
    for (int i = 0; i < 32; i += 8) {
        uint64_t x = prng64();
        memcpy(&data[i], &x, 8);
    }
}

static void analyze_buckets(const char *label, uint32_t buckets[BUCKETS]) {
    uint32_t min_b = UINT32_MAX, max_b = 0;
    for (int i = 0; i < BUCKETS; i++) {
        if (buckets[i] > max_b) max_b = buckets[i];
        if (buckets[i] < min_b) min_b = buckets[i];
    }

    double avg = (double)SAMPLES / BUCKETS;
    double stddev = 0.0;
    for (int i = 0; i < BUCKETS; i++) {
        double diff = buckets[i] - avg;
        stddev += diff * diff;
    }
    stddev = sqrt(stddev / BUCKETS);

    printf("  [%s]\n", label);
    printf("    Avg/bucket: %.2f\n", avg);
    printf("    Min/Max:    %u / %u\n", min_b, max_b);
    printf("    Stddev:     %.2f (threshold: %.2f) -> %s\n",
           stddev, COLLISION_STDDEV_THRESHOLD,
           stddev < COLLISION_STDDEV_THRESHOLD ? "PASS" : "FAIL");
}

// ---- 256-bit test ----

void test_collision_256(void) {
    printf("\n=== 256-bit: Distribution / Collision-ish (25M hashes) ===\n");

    uint64_t state[8];
    uint64_t hash[4];

    uint32_t buckets_word0_low[BUCKETS]  = {0};
    uint32_t buckets_word0_mid[BUCKETS]  = {0};
    uint32_t buckets_word0_high[BUCKETS] = {0};

    uint32_t buckets_word3_low[BUCKETS]  = {0};
    uint32_t buckets_word3_mid[BUCKETS]  = {0};
    uint32_t buckets_word3_high[BUCKETS] = {0};

    for (uint64_t i = 0; i < SAMPLES; i++) {
        if (i % BATCH == 0 && i > 0) {
            printf("  Progress: %llu/%llu\r",
                   (unsigned long long)i,
                   (unsigned long long)SAMPLES);
            fflush(stdout);
        }

        memset(state, 0, sizeof(state));

        uint8_t data[32];
        fill_random_32(data);

        sshasha256(state, i, data, 32);
        sshashs256(state, hash);

        uint64_t h0 = hash[0];
        uint64_t h3 = hash[3];

        uint32_t b0_low  = (uint32_t)( h0        & 0xFFFF);
        uint32_t b0_mid  = (uint32_t)((h0 >> 24) & 0xFFFF);
        uint32_t b0_high = (uint32_t)((h0 >> 48) & 0xFFFF);

        uint32_t b3_low  = (uint32_t)( h3        & 0xFFFF);
        uint32_t b3_mid  = (uint32_t)((h3 >> 24) & 0xFFFF);
        uint32_t b3_high = (uint32_t)((h3 >> 48) & 0xFFFF);

        buckets_word0_low[b0_low]++;
        buckets_word0_mid[b0_mid]++;
        buckets_word0_high[b0_high]++;

        buckets_word3_low[b3_low]++;
        buckets_word3_mid[b3_mid]++;
        buckets_word3_high[b3_high]++;
    }

    printf("  Progress: %llu/%llu\n",
           (unsigned long long)SAMPLES,
           (unsigned long long)SAMPLES);

    analyze_buckets("word0 low 16",  buckets_word0_low);
    analyze_buckets("word0 mid 16",  buckets_word0_mid);
    analyze_buckets("word0 high 16", buckets_word0_high);

    analyze_buckets("word3 low 16",  buckets_word3_low);
    analyze_buckets("word3 mid 16",  buckets_word3_mid);
    analyze_buckets("word3 high 16", buckets_word3_high);
}

// ---- 128-bit test ----

void test_collision_128(void) {
    printf("\n=== 128-bit: Distribution / Collision-ish (25M hashes) ===\n");

    uint32_t state[8];
    uint32_t hash[4];

    uint32_t buckets_word0_low[BUCKETS]  = {0};
    uint32_t buckets_word0_mid[BUCKETS]  = {0};
    uint32_t buckets_word0_high[BUCKETS] = {0};

    uint32_t buckets_word3_low[BUCKETS]  = {0};
    uint32_t buckets_word3_mid[BUCKETS]  = {0};
    uint32_t buckets_word3_high[BUCKETS] = {0};

    for (uint64_t i = 0; i < SAMPLES; i++) {
        if (i % BATCH == 0 && i > 0) {
            printf("  Progress: %llu/%llu\r",
                   (unsigned long long)i,
                   (unsigned long long)SAMPLES);
            fflush(stdout);
        }

        memset(state, 0, sizeof(state));

        uint8_t data[32];
        fill_random_32(data);

        sshasha128(state, (uint32_t)i, data, 32);
        sshashs128(state, hash);

        uint32_t h0 = hash[0];
        uint32_t h3 = hash[3];

        uint32_t b0_low  = (h0      ) & 0xFFFF;
        uint32_t b0_mid  = (h0 >> 8 ) & 0xFFFF;
        uint32_t b0_high = (h0 >> 16) & 0xFFFF;

        uint32_t b3_low  = (h3      ) & 0xFFFF;
        uint32_t b3_mid  = (h3 >> 8 ) & 0xFFFF;
        uint32_t b3_high = (h3 >> 16) & 0xFFFF;

        buckets_word0_low[b0_low]++;
        buckets_word0_mid[b0_mid]++;
        buckets_word0_high[b0_high]++;

        buckets_word3_low[b3_low]++;
        buckets_word3_mid[b3_mid]++;
        buckets_word3_high[b3_high]++;
    }

    printf("  Progress: %llu/%llu\n",
           (unsigned long long)SAMPLES,
           (unsigned long long)SAMPLES);

    analyze_buckets("word0 low 16",  buckets_word0_low);
    analyze_buckets("word0 mid 16",  buckets_word0_mid);
    analyze_buckets("word0 high 16", buckets_word0_high);

    analyze_buckets("word3 low 16",  buckets_word3_low);
    analyze_buckets("word3 mid 16",  buckets_word3_mid);
    analyze_buckets("word3 high 16", buckets_word3_high);
}

void test_avalanche_256(void) {
    printf("\n=== 256-bit: Avalanche Effect (25M mutations) ===\n");

    uint64_t total_flips = 0;
    uint32_t weak_count = 0;

    for (uint64_t i = 0; i < SAMPLES; i++) {
        if (i % BATCH == 0 && i > 0) {
            printf("  Progress: %llu/25000000\r", i);
            fflush(stdout);
        }

        uint64_t state1[8] = {0}, state2[8] = {0};
        uint64_t hash1[4], hash2[4];
        uint8_t data1[64], data2[64];

        for (int j = 0; j < 64; j++) {
            data1[j] = (rand() >> (j % 3)) & 0xFF;
        }
        memcpy(data2, data1, 64);

        int byte_pos = rand() % 64;
        int bit_pos = rand() % 8;
        data2[byte_pos] ^= (1 << bit_pos);

        sshasha256(state1, 0, data1, 64);
        sshashs256(state1, hash1);

        sshasha256(state2, 0, data2, 64);
        sshashs256(state2, hash2);

        uint32_t bit_diff = 0;
        for (int j = 0; j < 4; j++) {
            uint64_t xor = hash1[j] ^ hash2[j];
            bit_diff += __builtin_popcountll(xor);
        }

        total_flips += bit_diff;
        if (bit_diff < 90 || bit_diff > 170) weak_count++;
    }

    printf("  Progress: 25000000/25000000\n");
    double avg = (double)total_flips / SAMPLES;
    printf("Avg bits flipped: %.2f/256 (ideal: 128)\n", avg);
    double weak_pct = (double)weak_count * 100 / SAMPLES;
    printf("Weak cases: %u (%.3f%%)\n", weak_count, weak_pct);
    printf("Status: %s\n", avg > 110 && avg < 150 && weak_pct < WEAK_PERCENT_THRESHOLD ? "PASS" : "FAIL");
}

void test_avalanche_128(void) {
    printf("\n=== 128-bit: Avalanche Effect (25M mutations) ===\n");

    uint64_t total_flips = 0;
    uint32_t weak_count = 0;

    for (uint64_t i = 0; i < SAMPLES; i++) {
        if (i % BATCH == 0 && i > 0) {
            printf("  Progress: %llu/25000000\r", i);
            fflush(stdout);
        }

        uint32_t state1[8] = {0}, state2[8] = {0};
        uint32_t hash1[4], hash2[4];
        uint8_t data1[64], data2[64];

        for (int j = 0; j < 64; j++) {
            data1[j] = (rand() >> (j % 3)) & 0xFF;
        }
        memcpy(data2, data1, 64);

        int byte_pos = rand() % 64;
        int bit_pos = rand() % 8;
        data2[byte_pos] ^= (1 << bit_pos);

        sshasha128(state1, 0, data1, 64);
        sshashs128(state1, hash1);

        sshasha128(state2, 0, data2, 64);
        sshashs128(state2, hash2);

        uint32_t bit_diff = 0;
        for (int j = 0; j < 4; j++) {
            uint32_t xor = hash1[j] ^ hash2[j];
            bit_diff += __builtin_popcount(xor);
        }

        total_flips += bit_diff;
        if (bit_diff < 45 || bit_diff > 85) weak_count++;
    }

    printf("  Progress: 25000000/25000000\n");
    double avg_128 = (double)total_flips / SAMPLES;
    printf("Avg bits flipped: %.2f/128 (ideal: 64)\n", avg_128);
    double weak_pct_128 = (double)weak_count * 100 / SAMPLES;
    printf("Weak cases: %u (%.3f%%)\n", weak_count, weak_pct_128);
    printf("Status: %s\n", avg_128 > 55 && avg_128 < 75 && weak_pct_128 < WEAK_PERCENT_THRESHOLD ? "PASS" : "FAIL");
}

void test_distribution_256(void) {
    printf("\n=== 256-bit: Output Distribution (25M samples) ===\n");

    uint64_t byte_freq[256] = {0};
    uint64_t bit_freq[8] = {0};

    for (uint64_t i = 0; i < SAMPLES; i++) {
        if (i % BATCH == 0 && i > 0) {
            printf("  Progress: %llu/25000000\r", i);
            fflush(stdout);
        }

        uint64_t state[8] = {0};
        uint64_t hash[4];
        uint8_t data[16];
        for (int j = 0; j < 16; j++) data[j] = (i >> (j % 8)) & 0xFF;

        sshasha256(state, i, data, 16);
        sshashs256(state, hash);

        uint8_t* bytes = (uint8_t*)hash;
        for (int j = 0; j < 32; j++) {
            byte_freq[bytes[j]]++;
            for (int b = 0; b < 8; b++) {
                if (bytes[j] & (1 << b)) bit_freq[b]++;
            }
        }
    }

    printf("  Progress: 25000000/25000000\n");

    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (byte_freq[i] > 0) {
            double p = (double)byte_freq[i] / (SAMPLES * 32);
            entropy -= p * log2(p);
        }
    }

    double expected = (double)(SAMPLES * 32) / 256;
    double chi2 = 0.0;
    for (int i = 0; i < 256; i++) {
        double diff = (double)byte_freq[i] - expected;
        chi2 += (diff * diff) / expected;
    }

    printf("Entropy: %.4f/8.0 bits\n", entropy);
    printf("Chi-square: %.2f (critical: ~293)\n", chi2);
    printf("Status: %s\n", entropy > 7.98 && chi2 < 350 ? "PASS" : "FAIL");
}

void test_distribution_128(void) {
    printf("\n=== 128-bit: Output Distribution (25M samples) ===\n");

    uint64_t byte_freq[256] = {0};

    for (uint64_t i = 0; i < SAMPLES; i++) {
        if (i % BATCH == 0 && i > 0) {
            printf("  Progress: %llu/25000000\r", i);
            fflush(stdout);
        }

        uint32_t state[8] = {0};
        uint32_t hash[4];
        uint8_t data[16];
        for (int j = 0; j < 16; j++) data[j] = (i >> (j % 8)) & 0xFF;

        sshasha128(state, i, data, 16);
        sshashs128(state, hash);

        uint8_t* bytes = (uint8_t*)hash;
        for (int j = 0; j < 16; j++) {
            byte_freq[bytes[j]]++;
        }
    }

    printf("  Progress: 25000000/25000000\n");

    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (byte_freq[i] > 0) {
            double p = (double)byte_freq[i] / (SAMPLES * 16);
            entropy -= p * log2(p);
        }
    }

    double expected = (double)(SAMPLES * 16) / 256;
    double chi2 = 0.0;
    for (int i = 0; i < 256; i++) {
        double diff = (double)byte_freq[i] - expected;
        chi2 += (diff * diff) / expected;
    }

    printf("Entropy: %.4f/8.0 bits\n", entropy);
    printf("Chi-square: %.2f (critical: ~293)\n", chi2);
    printf("Status: %s\n", entropy > 7.98 && chi2 < 350 ? "PASS" : "FAIL");
}

void test_patterns_256(void) {
    printf("\n=== 256-bit: Pattern Differential (25M tests) ===\n");

    uint8_t patterns[] = {0x00, 0xFF, 0xAA, 0x55};
    uint32_t weak_patterns = 0;

    for (int p = 0; p < 4; p++) {
        for (uint64_t i = 0; i < SAMPLES/4; i++) {
            if (i % BATCH == 0 && i > 0) {
                printf("  Pattern %d, Progress: %llu/6250000\r", p, i);
                fflush(stdout);
            }

            uint64_t state1[8] = {0}, state2[8] = {0};
            uint64_t hash1[4], hash2[4];
            uint8_t data1[32], data2[32];

            memset(data1, patterns[p], 32);
            memset(data2, patterns[p] ^ 0xFF, 32);

            sshasha256(state1, 0, data1, 32);
            sshashs256(state1, hash1);

            sshasha256(state2, 0, data2, 32);
            sshashs256(state2, hash2);

            uint64_t diff_bits = 0;
            for (int j = 0; j < 4; j++) {
                uint64_t xor = hash1[j] ^ hash2[j];
                diff_bits += __builtin_popcountll(xor);
            }

            if (diff_bits < 100) weak_patterns++;
        }
    }

    printf("  Pattern done                          \n");
    printf("Tests: 25000000\n");
    double weak_pct_pat = (double)weak_patterns * 100 / SAMPLES;
    printf("Weak patterns: %u (%.3f%%)\n", weak_patterns, weak_pct_pat);
    printf("Status: %s\n", weak_pct_pat < WEAK_PERCENT_THRESHOLD ? "PASS" : "FAIL");
}

void test_patterns_128(void) {
    printf("\n=== 128-bit: Pattern Differential (25M tests) ===\n");

    uint8_t patterns[] = {0x00, 0xFF, 0xAA, 0x55};
    uint32_t weak_patterns = 0;

    for (int p = 0; p < 4; p++) {
        for (uint64_t i = 0; i < SAMPLES/4; i++) {
            if (i % BATCH == 0 && i > 0) {
                printf("  Pattern %d, Progress: %llu/6250000\r", p, i);
                fflush(stdout);
            }

            uint32_t state1[8] = {0}, state2[8] = {0};
            uint32_t hash1[4], hash2[4];
            uint8_t data1[32], data2[32];

            memset(data1, patterns[p], 32);
            memset(data2, patterns[p] ^ 0xFF, 32);

            sshasha128(state1, 0, data1, 32);
            sshashs128(state1, hash1);

            sshasha128(state2, 0, data2, 32);
            sshashs128(state2, hash2);

            uint32_t diff_bits = 0;
            for (int j = 0; j < 4; j++) {
                uint32_t xor = hash1[j] ^ hash2[j];
                diff_bits += __builtin_popcount(xor);
            }

            if (diff_bits < 50) weak_patterns++;
        }
    }

    printf("  Pattern done                          \n");
    printf("Tests: 25000000\n");
    printf("Weak patterns: %u (%.3f%%)\n", weak_patterns,
           (double)weak_patterns * 100 / SAMPLES);
    printf("Status: %s\n", weak_patterns < 100000 ? "PASS" : "FAIL");
}

void test_counter_256(void) {
    printf("\n=== 256-bit: Counter Properties (25M iterations) ===\n");

    uint32_t weak_counter = 0;

    for (uint64_t ctr = 0; ctr < SAMPLES; ctr++) {
        if (ctr % BATCH == 0 && ctr > 0) {
            printf("  Progress: %llu/25000000\r", ctr);
            fflush(stdout);
        }

        uint64_t state1[8] = {0}, state2[8] = {0};
        uint64_t hash1[4], hash2[4];
        uint8_t data[32];
        memset(data, 0xCC, 32);

        sshasha256(state1, ctr, data, 32);
        sshashs256(state1, hash1);

        memcpy(state2, (uint64_t[8]){0}, sizeof(state2));
        sshasha256(state2, ctr + 1, data, 32);
        sshashs256(state2, hash2);

        uint32_t diff = 0;
        for (int i = 0; i < 4; i++) {
            uint64_t xor = hash1[i] ^ hash2[i];
            diff += __builtin_popcountll(xor);
        }

        if (diff < 80) weak_counter++;
    }

    printf("  Progress: 25000000/25000000\n");
    printf("Weak cases: %u (%.3f%%)\n", weak_counter,
           (double)weak_counter * 100 / SAMPLES);
    printf("Status: %s\n", weak_counter < 100000 ? "PASS" : "FAIL");
}

void test_counter_128(void) {
    printf("\n=== 128-bit: Counter Properties (25M iterations) ===\n");

    uint32_t weak_counter = 0;

    for (uint64_t ctr = 0; ctr < SAMPLES; ctr++) {
        if (ctr % BATCH == 0 && ctr > 0) {
            printf("  Progress: %llu/25000000\r", ctr);
            fflush(stdout);
        }

        uint32_t state1[8] = {0}, state2[8] = {0};
        uint32_t hash1[4], hash2[4];
        uint8_t data[32];
        memset(data, 0xCC, 32);

        sshasha128(state1, ctr, data, 32);
        sshashs128(state1, hash1);

        memcpy(state2, (uint32_t[8]){0}, sizeof(state2));
        sshasha128(state2, ctr + 1, data, 32);
        sshashs128(state2, hash2);

        uint32_t diff = 0;
        for (int i = 0; i < 4; i++) {
            uint32_t xor = hash1[i] ^ hash2[i];
            diff += __builtin_popcount(xor);
        }

        if (diff < 40) weak_counter++;
    }

    printf("  Progress: 25000000/25000000\n");
    printf("Weak cases: %u (%.3f%%)\n", weak_counter,
           (double)weak_counter * 100 / SAMPLES);
    printf("Status: %s\n", weak_counter < 100000 ? "PASS" : "FAIL");
}

int main(void) {
    srand((unsigned)time(NULL));

    printf("\n=========================================\n");
    printf("sshash Security Analysis - 25M Samples\n");
    printf("=========================================\n");

    test_collision_256();
    test_collision_128();

    test_avalanche_256();
    test_avalanche_128();

    test_distribution_256();
    test_distribution_128();

    test_patterns_256();
    test_patterns_128();

    test_counter_256();
    test_counter_128();

    printf("\n=========================================\n");
    printf("Analysis Complete\n");
    printf("=========================================\n\n");

    return 0;
}