#ifndef LZW_H
#define LZW_H

#include <stdio.h>
#include <stdint.h>

#define MAX_BITS 12
#define MAX_DICT_SIZE (1 << MAX_BITS) /* 4096 */
#define CODE_CLEAR 256
#define CODE_END   257
#define CODE_FIRST 258 /* first free code */

typedef struct {
    FILE *f;
    uint32_t bitbuf;
    int bitcount;
} BitWriter;

typedef struct {
    FILE *f;
    uint32_t bitbuf;
    int bitcount;
    int eof;
} BitReader;

/* interfaces */
int compress_file(const char *infile, const char *outfile);
int decompress_file(const char *infile, const char *outfile);

/* bit writer/reader */
void bw_init(BitWriter *bw, FILE *f);
void bw_write_code(BitWriter *bw, unsigned int code);
void bw_flush(BitWriter *bw);

void br_init(BitReader *br, FILE *f);
int br_read_code(BitReader *br); /* returns -1 on EOF / no full code */

#endif /* LZW_H */
