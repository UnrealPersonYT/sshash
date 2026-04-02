#include <sshash.h>
#include <string.h>
#include <stdio.h>

enum{
    MODE_128,
    MODE_256
};
enum{
    MODE_HEX,
    MODE_RAW
};

uint8_t inp_buf[4096] = {0};
uint8_t hash[32] = {0};

int main(int argc, char** argv){
    int mode = MODE_128, pmode = MODE_HEX;

    --argc;++argv;  // Skip cstdlib
    if(argc){
        if(!strcmp(*argv, "--help")){
            puts(
                "Usage: sshash [--128 | --256] [--hex | --raw]\n"
                "\n"
                "Options:\n"
                "  --128       Output a 128-bit hash (default)\n"
                "  --256       Output a 256-bit hash\n"
                "  --hex       Print hash in hexadecimal (default)\n"
                "  --raw       Print raw binary hash\n"
                "  --help      Show this message"
            );
            return 0;
        }
        while(argc--){
            if(!strcmp(*argv, "--128")){
                mode = MODE_128;
            }else if(!strcmp(*argv, "--256")){
                mode = MODE_256;
            }else if(!strcmp(*argv, "--hex")){
                pmode = MODE_HEX;
            }else if(!strcmp(*argv, "--raw"))
                pmode = MODE_RAW;
            ++argv;
        }
    }

    if(mode == MODE_128){
        size_t acc = 0, read;
        uint32_t ctr = 0, state[8] = {0};
        while((read = fread(&inp_buf[acc], sizeof(char), sizeof(inp_buf) - acc, stdin))){
            acc += read;
            if(acc == sizeof(inp_buf)){
                ctr = sshasha128(state, ctr, inp_buf, sizeof(inp_buf));
                acc = 0;
            }
        }
        if(ferror(stdin))
            return 1;
        (void)sshasha128(state, ctr, inp_buf, acc);
        sshashs128(state, (uint32_t*)hash);
    }else{
        size_t acc = 0, read;
        uint64_t ctr = 0, state[8] = {0};
        while((read = fread(&inp_buf[acc], sizeof(char), sizeof(inp_buf) - acc, stdin))){
            acc += read;
            if(acc == sizeof(inp_buf)){
                ctr = sshasha256(state, ctr, inp_buf, sizeof(inp_buf));
                acc = 0;
            }
        }
        if(ferror(stdin))
            return 1;
        (void)sshasha256(state, ctr, inp_buf, acc);
        sshashs256(state, (uint64_t*)hash);
    }

    if(pmode == MODE_HEX){
        printf("0x");
        for(size_t i = 0; i < (size_t)(16 + (16 * mode)); ++i)
            printf("%02X", hash[i]);
    }else
        fwrite(hash, sizeof(char), (16 + (16 * mode)), stdout);

    return 0;
}