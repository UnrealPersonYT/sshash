#ifndef SSHASH_H
#define SSHASH_H
/// @file  sshash.h
/// @brief Contains the sshash API

#include <stdint.h>
#include <stddef.h>

/// @brief         Absorbs `data` into a 512-bit `state` with a given `ctr` for `datalen` bytes
/// @param state   512-bit state
/// @param ctr     Counter for destroying repeating/zeroed `data`
/// @param data    Input data to absorb
/// @param datalen Total length of `data` in bytes
/// @return        New `ctr`
uint64_t sshasha256(uint64_t state[8], uint64_t ctr, const void* data, size_t datalen);

/// @brief       Squeezes a 512-bit `state` into a 256-bit `hash`
/// @param state 512-bit state
/// @param hash  256-bit hash
void sshashs256(uint64_t state[8], uint64_t hash[4]);

/// @brief         Absorbs `data` into a 256-bit `state` with a given `ctr` for `datalen` bytes
/// @param state   256-bit state
/// @param ctr     Counter for destroying repeating/zeroed `data`
/// @param data    Input data to hash
/// @param datalen Total length of `data` in bytes
/// @return        New `ctr`
uint32_t sshasha128(uint32_t state[8], uint32_t ctr, const void* data, size_t datalen);

/// @brief       Squeezes a 256-bit `state` into a 128-bit `hash`
/// @param state 256-bit state
/// @param hash  128-bit hash
void sshashs128(uint32_t state[8], uint32_t hash[4]);

#endif  // SSHASH_H