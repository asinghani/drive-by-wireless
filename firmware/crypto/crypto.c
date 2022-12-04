#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "time.h"
#include "hardware/regs/rosc.h"
#include "hardware/regs/addressmap.h"
#include "rfc7539.h"
#include "crypto.h"

#define DATA_SIZE 64
#define NONCE_SIZE 12
#define TAG_SIZE 15
#define MESSAGE_SIZE (DATA_SIZE + NONCE_SIZE + TAG_SIZE)

uint8_t shared_enc_key[32] = {
    0xd7, 0x13, 0xb6, 0x04, 0x80, 0xd3, 0xbc, 0xc0, 0xe9, 0xa7, 0x1d, 0xf0, 0x54, 0x45, 0xe8, 0x35,
    0x9f, 0x02, 0x64, 0x5e, 0xa4, 0x26, 0x62, 0xbd, 0x9a, 0x33, 0x95, 0x04, 0xb4, 0x1f, 0x27, 0x6c
};

chacha20poly1305_ctx ctx;

void crypto_init() {
    // Seed randomness using ROSC noise
    // https://forums.raspberrypi.com/viewtopic.php?f=145&t=302960#p1814787
    unsigned int random = 0x811c9dc5;
    uint8_t next_byte = 0;
    volatile unsigned int *rnd_reg = (unsigned int *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);
    for (int i = 0; i < 16; i++) {
        for (int k = 0; k < 8; k++) {
            next_byte = (next_byte << 1) | (*rnd_reg & 1);
        }
        random ^= next_byte;
        random *= 0x01000193;
    }
    srand(random);
}

void crypto_encrypt(uint8_t *plaintext, uint8_t *ciphertext) {
    uint8_t *message_nonce = ciphertext;
    uint8_t *message_data = &ciphertext[NONCE_SIZE];
    uint8_t *message_tag = &ciphertext[NONCE_SIZE + DATA_SIZE];

    for (int i = 0; i < NONCE_SIZE; i++) {
        message_nonce[i] = rand() % 255;
    }

    rfc7539_init(&ctx, shared_enc_key, message_nonce);
    chacha20poly1305_encrypt(&ctx, plaintext, message_data, DATA_SIZE);
    rfc7539_finish(&ctx, 0, DATA_SIZE, message_tag);
    memset(&ctx, 0, sizeof(ctx));
}


bool crypto_decrypt(uint8_t *ciphertext, uint8_t *plaintext) {
    uint8_t *message_nonce = ciphertext;
    uint8_t *message_data = &ciphertext[NONCE_SIZE];
    uint8_t *message_tag = &ciphertext[NONCE_SIZE + DATA_SIZE];
    uint8_t decrypted_tag[TAG_SIZE];

    rfc7539_init(&ctx, shared_enc_key, message_nonce);
    chacha20poly1305_decrypt(&ctx, message_data, plaintext, DATA_SIZE);
    rfc7539_finish(&ctx, 0, DATA_SIZE, decrypted_tag);
    memset(&ctx, 0, sizeof(ctx));
    return !(memcmp(message_tag, decrypted_tag, TAG_SIZE));
}
