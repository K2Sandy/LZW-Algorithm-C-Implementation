#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzw.h"

int compress_file(const char *infile, const char *outfile);
int decompress_file(const char *infile, const char *outfile);

int main(void) {
    char pilihan[8];
    char inname[512];
    char outname[512];

    while (1) {
        printf("\n=== LZW (GIF-style fixed 12-bit) ===\n");
        printf("1. Compress file\n");
        printf("2. Decompress file\n");
        printf("3. Exit\n");
        printf("Pilih (1/2/3): ");
        if (fgets(pilihan, sizeof(pilihan), stdin) == NULL) break;

        if (pilihan[0] == '3') {
            printf("Keluar.\n");
            break;
        } else if (pilihan[0] == '1') {
            printf("Nama file input  : ");
            if (fgets(inname, sizeof(inname), stdin) == NULL) break;
            inname[strcspn(inname, "\r\n")] = 0;
            printf("Nama file output : ");
            if (fgets(outname, sizeof(outname), stdin) == NULL) break;
            outname[strcspn(outname, "\r\n")] = 0;

            printf("Mengompres %s -> %s ...\n", inname, outname);
            int r = compress_file(inname, outname);
            if (r == 0) printf("Selesai: file disimpan ke %s\n", outname);
            else printf("Gagal mengompres (kode %d)\n", r);
        } else if (pilihan[0] == '2') {
            printf("Nama file input  : ");
            if (fgets(inname, sizeof(inname), stdin) == NULL) break;
            inname[strcspn(inname, "\r\n")] = 0;
            printf("Nama file output : ");
            if (fgets(outname, sizeof(outname), stdin) == NULL) break;
            outname[strcspn(outname, "\r\n")] = 0;

            printf("Mendekompres %s -> %s ...\n", inname, outname);
            int r = decompress_file(inname, outname);
            if (r == 0) printf("Selesai: file disimpan ke %s\n", outname);
            else printf("Gagal mendekompres (kode %d)\n", r);
        } else {
            printf("Pilihan tidak valid.\n");
        }
    }

    return 0;
}
