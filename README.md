# sshash

A fast, lightweight hashing library written in C.

## Overview

sshash is a custom hashing algorithm implementation that provides both 256-bit and 128-bit hash variants. It uses a sponge construction with a permutation-based design.

## Features

- **256-bit hashing**: Using a 512-bit internal state
- **128-bit hashing**: Using a 256-bit internal state
- **Lightweight**: Minimal dependencies, simple and fast
- **Endianness support**: Handles both big-endian and little-endian systems

## API

### 256-bit Hash Functions

```c
uint64_t sshasha256(uint64_t state[8], uint64_t ctr, const void* data, size_t datalen);
void sshashs256(uint64_t state[8], uint64_t hash[4]);
```

- `sshasha256`: Absorbs data into the 512-bit state. Returns updated counter for chaining.
- `sshashs256`: Squeezes the state to produce a 256-bit hash.

### 128-bit Hash Functions

```c
uint32_t sshasha128(uint32_t state[8], uint32_t ctr, const void* data, size_t datalen);
void sshashs128(uint32_t state[8], uint32_t hash[4]);
```

- `sshasha128`: Absorbs data into the 256-bit state. Returns updated counter for chaining.
- `sshashs128`: Squeezes the state to produce a 128-bit hash.

### Compilation

Create output directories:
```bash
mkdir -p lib bin ctests
```

Compile the library:
```bash
gcc -Wall -Wextra -Wpedantic -Werror -O3 -I./include -c src/sshash.c -o lib/sshash.o
ar rcs lib/libsshash.a lib/sshash.o
rm lib/sshash.o
```

Build the CLI tool:
```bash
gcc -Wall -Wextra -Wpedantic -Werror -O3 -I./include -L./lib -s -o bin/sshash tools/sshash.c -lsshash
```

Build speed test:
```bash
gcc -Wall -Wextra -Wpedantic -Werror -O3 -I./include -L./lib -g -o ctests/speed tests/speed.c -lsshash
```

Build security test:
```bash
gcc -Wall -Wextra -Wpedantic -Werror -O3 -I./include -L./lib -g -o ctests/security tests/security.c -lsshash
```

Run tests:
```bash
./ctests/test_speed
./ctests/test_security
```

## Directory Structure

- `src/` - Source code
- `include/` - Public headers
- `lib/` - Compiled library
- `tests/` - Test suite
- `ctests/` - C-specific tests
- `bin/` - Compiled binaries
- `tools/` - Utility tools

## License

[LICENSE](LICENSE)