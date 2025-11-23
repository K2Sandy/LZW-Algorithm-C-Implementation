#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzw.h"

/* Simple open-addressing hash table for (prefix, ch) -> code */
#define HASH_SIZE 16384 /* power of two, larger to reduce collisions */

typedef struct {
    int prefix;          /* -1 for single char */
    unsigned char ch;
    int code;
    int used;
} HashEntry;

static HashEntry hash_table[HASH_SIZE];

static inline unsigned int _hash_key(int prefix, unsigned char ch) {
    /* simple mix */
    uint32_t k = ((uint32_t)prefix << 8) ^ (uint32_t)ch;
    k ^= k >> 13;
    k *= 0x5bd1e995;
    k ^= k >> 15;
    return (unsigned int)k & (HASH_SIZE - 1);
}

static void hash_init(void) {
    for (int i = 0; i < HASH_SIZE; ++i) hash_table[i].used = 0;
}

static int hash_find(int prefix, unsigned char ch) {
    unsigned int idx = _hash_key(prefix, ch);
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry *e = &hash_table[idx];
        if (!e->used) return -1;
        if (e->prefix == prefix && e->ch == ch) return e->code;
        idx = (idx + 1) & (HASH_SIZE - 1);
    }
    return -1;
}

static int hash_add(int prefix, unsigned char ch, int code) {
    unsigned int idx = _hash_key(prefix, ch);
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry *e = &hash_table[idx];
        if (!e->used) {
            e->used = 1;
            e->prefix = prefix;
            e->ch = ch;
            e->code = code;
            return 0;
        }
        idx = (idx + 1) & (HASH_SIZE - 1);
    }
    return -1;
}

/* BitWriter: little-endian packing (append codes LSB-first into stream) */
/* Example: store codes using 12-bit chunks into a byte stream reliably */
void bw_init(BitWriter *bw, FILE *f) {
    bw->f = f;
    bw->bitbuf = 0;
    bw->bitcount = 0;
}

void bw_write_code(BitWriter *bw, unsigned int code) {
    const unsigned int mask = (1u << MAX_BITS) - 1u;
    bw->bitbuf |= ( (uint32_t)(code & mask) << bw->bitcount );
    bw->bitcount += MAX_BITS;

    while (bw->bitcount >= 8) {
        unsigned char byte = (unsigned char)(bw->bitbuf & 0xFFu);
        if (fputc(byte, bw->f) == EOF) {
            /* ignore write error handling here */
        }
        bw->bitbuf >>= 8;
        bw->bitcount -= 8;
    }
}

void bw_flush(BitWriter *bw) {
    while (bw->bitcount > 0) {
        unsigned char byte = (unsigned char)(bw->bitbuf & 0xFFu);
        if (fputc(byte, bw->f) == EOF) { }
        bw->bitbuf >>= 8;
        bw->bitcount -= 8;
    }
    bw->bitbuf = 0;
    bw->bitcount = 0;
}

/* compress_file: LZW GIF-style (fixed 12-bit) with CLEAR/END handling */
int compress_file(const char *infile, const char *outfile) {
    FILE *inf = fopen(infile, "rb");
    if (!inf) { perror("Buka input gagal"); return 1; }
    FILE *outf = fopen(outfile, "wb");
    if (!outf) { perror("Buka output gagal"); fclose(inf); return 1; }

    BitWriter bw;
    bw_init(&bw, outf);

    /* initialize dictionary */
    hash_init();
    for (int i = 0; i < 256; ++i) {
        hash_add(-1, (unsigned char)i, i);
    }
    int next_code = CODE_FIRST; /* 258 */

    int c = fgetc(inf);
    if (c == EOF) {
        /* empty file: write END and finish */
        bw_write_code(&bw, CODE_END);
        bw_flush(&bw);
        fclose(inf);
        fclose(outf);
        return 0;
    }

    int prefix = c;

    while ((c = fgetc(inf)) != EOF) {
        unsigned char ch = (unsigned char)c;
        int found = hash_find(prefix, ch);
        if (found != -1) {
            prefix = found;
        } else {
            /* output prefix code */
            bw_write_code(&bw, (unsigned int)prefix);

            /* add new entry if possible */
            if (next_code < MAX_DICT_SIZE) {
                hash_add(prefix, ch, next_code++);
            } else {
                /* dictionary full: emit CLEAR and reset */
                bw_write_code(&bw, CODE_CLEAR);
                hash_init();
                for (int i = 0; i < 256; ++i) hash_add(-1, (unsigned char)i, i);
                next_code = CODE_FIRST;
            }
            prefix = ch;
        }
    }

    /* write last prefix */
    bw_write_code(&bw, (unsigned int)prefix);
    /* write END */
    bw_write_code(&bw, CODE_END);
    bw_flush(&bw);

    fclose(inf);
    fclose(outf);
    return 0;
}
