#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzw.h"

/* BitReader matching little-endian packing used by BitWriter */
void br_init(BitReader *br, FILE *f) {
    br->f = f;
    br->bitbuf = 0;
    br->bitcount = 0;
    br->eof = 0;
}

/* read one code (12-bit) or return -1 if no full code available */
int br_read_code(BitReader *br) {
    const unsigned int mask = (1u << MAX_BITS) - 1u;
    while (br->bitcount < MAX_BITS && !br->eof) {
        int byte = fgetc(br->f);
        if (byte == EOF) {
            br->eof = 1;
            break;
        }
        br->bitbuf |= ((uint32_t)(unsigned char)byte << br->bitcount);
        br->bitcount += 8;
    }
    if (br->bitcount < MAX_BITS) return -1;
    int code = (int)(br->bitbuf & mask);
    br->bitbuf >>= MAX_BITS;
    br->bitcount -= MAX_BITS;
    return code;
}

/* Decompress function with table of malloc'd strings */
int decompress_file(const char *infile, const char *outfile) {
    FILE *inf = fopen(infile, "rb");
    if (!inf) { perror("Buka input gagal"); return 1; }
    FILE *outf = fopen(outfile, "wb");
    if (!outf) { perror("Buka output gagal"); fclose(inf); return 1; }

    char *table[MAX_DICT_SIZE];
    int table_len[MAX_DICT_SIZE];
    for (int i = 0; i < MAX_DICT_SIZE; ++i) { table[i] = NULL; table_len[i] = 0; }

    /* init single-byte entries */
    for (int i = 0; i < 256; ++i) {
        table[i] = (char*)malloc(1);
        table[i][0] = (char)i;
        table_len[i] = 1;
    }
    int next_code = CODE_FIRST; /* 258 */

    BitReader br;
    br_init(&br, inf);

    int prev_code = br_read_code(&br);
    if (prev_code == -1) {
        /* nothing to do */
        fclose(inf); fclose(outf);
        for (int i = 0; i < next_code; ++i) if (table[i]) free(table[i]);
        return 0;
    }

    /* handle possible CLEAR at start (rare) */
    if (prev_code == CODE_CLEAR) {
        for (int i = 256; i < next_code; ++i) { if (table[i]) { free(table[i]); table[i] = NULL; } }
        next_code = CODE_FIRST;
        prev_code = br_read_code(&br);
        if (prev_code == -1) { fclose(inf); fclose(outf); return 0; }
    }
    if (prev_code == CODE_END) {
        /* immediate end */
        fclose(inf); fclose(outf);
        for (int i = 0; i < next_code; ++i) if (table[i]) free(table[i]);
        return 0;
    }

    /* write first entry */
    fwrite(table[prev_code], 1, table_len[prev_code], outf);

    int cur_code;
    while ((cur_code = br_read_code(&br)) != -1) {
        if (cur_code == CODE_CLEAR) {
            /* reset dictionary */
            for (int i = 256; i < next_code; ++i) { if (table[i]) { free(table[i]); table[i] = NULL; } }
            next_code = CODE_FIRST;
            /* read next code to continue */
            cur_code = br_read_code(&br);
            if (cur_code == -1 || cur_code == CODE_END) break;
            /* write entry for cur_code */
            if (cur_code < next_code && table[cur_code] != NULL) {
                fwrite(table[cur_code], 1, table_len[cur_code], outf);
                prev_code = cur_code;
                continue;
            } else {
                /* invalid sequence after CLEAR */
                break;
            }
        } else if (cur_code == CODE_END) {
            break;
        }

        char *entry = NULL;
        int entry_len = 0;

        if (cur_code < next_code && table[cur_code] != NULL) {
            /* normal case */
            entry_len = table_len[cur_code];
            entry = (char*)malloc(entry_len);
            memcpy(entry, table[cur_code], entry_len);
        } else if (cur_code == next_code) {
            /* special case: K = W + W[0] */
            entry_len = table_len[prev_code] + 1;
            entry = (char*)malloc(entry_len);
            memcpy(entry, table[prev_code], table_len[prev_code]);
            entry[entry_len - 1] = table[prev_code][0];
        } else {
            fprintf(stderr, "Error: kode tidak valid selama dekompresi: %d\n", cur_code);
            break;
        }

        /* write entry to output */
        fwrite(entry, 1, entry_len, outf);

        /* add new table entry: prev + first_char(entry) */
        if (next_code < MAX_DICT_SIZE) {
            int newlen = table_len[prev_code] + 1;
            char *newstr = (char*)malloc(newlen);
            memcpy(newstr, table[prev_code], table_len[prev_code]);
            newstr[newlen - 1] = entry[0];
            table[next_code] = newstr;
            table_len[next_code] = newlen;
            next_code++;
        } else {
            /* dictionary full: encoder should have emitted CLEAR */
        }

        free(entry);
        prev_code = cur_code;
    }

    /* cleanup */
    for (int i = 0; i < MAX_DICT_SIZE; ++i) if (table[i]) { free(table[i]); table[i] = NULL; }

    fclose(inf);
    fclose(outf);
    return 0;
}
