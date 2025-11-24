#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lzw.h"

// Batas string untuk mencegah buffer overflow pada array tetap
#define MAX_STRING_LEN 1023 

static int findString(char **dict, int size, const char *str) {
    int len = (int)strlen(str);
    for (int i = 0; i < size; i++) {
        if (dict[i] && (int)strlen(dict[i]) == len && strcmp(dict[i], str) == 0)
            return i;
    }
    return -1;
}

void compressFile(const char *input, const char *output) {
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan!\n"); return; }

    FILE *out = fopen(output, "wb");
    if (!out) { printf("Gagal membuka file output!\n"); fclose(in); return; }

    char *dict[MAX_DICT_SIZE] = {0};
    int dictSize = 0;

    // Inisialisasi Kamus
    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {
        dict[i] = malloc(2); // Ukuran 1 karakter + \0
        if (!dict[i]) { printf("Error: malloc gagal\n"); /* cleanup */ 
            for (int j = 0; j < i; j++) free(dict[j]);
            fclose(in); fclose(out);
            return;
        }
        dict[i][0] = (char)i;
        dict[i][1] = '\0';
        dictSize++;
    }

    int c = fgetc(in);
    if (c == EOF) {
        // Bebaskan kamus sebelum keluar
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in);
        fclose(out);
        return;
    }

    char w[MAX_STRING_LEN + 1] = {0};
    w[0] = (char)c;
    w[1] = '\0';

    while ((c = fgetc(in)) != EOF) {
        char wc[MAX_STRING_LEN + 1];
        
        // Pengecekan batas untuk wc
        if (strlen(w) >= MAX_STRING_LEN) {
            // String w sudah terlalu panjang untuk array wc, kompres w sekarang
            int code = findString(dict, dictSize, w);
            uint16_t outcode = (uint16_t)code;
            fwrite(&outcode, sizeof(uint16_t), 1, out);

            // Reset w menjadi karakter c saat ini
            w[0] = (char)c;
            w[1] = '\0';
            continue; // Lanjutkan loop
        }
        
        // Buat string wc
        snprintf(wc, sizeof(wc), "%s%c", w, (char)c);

        if (findString(dict, dictSize, wc) != -1) {
            // wc ada di kamus, perbarui w
            strcpy(w, wc);
        } else {
            // wc tidak ada di kamus, kirim kode untuk w
            int code = findString(dict, dictSize, w);
            uint16_t outcode = (uint16_t)code;
            fwrite(&outcode, sizeof(uint16_t), 1, out);

            // Tambahkan wc ke kamus jika belum penuh
            if (dictSize < MAX_DICT_SIZE) {
                // Perbaikan: Alokasi memori yang benar
                dict[dictSize] = malloc(strlen(wc) + 1);
                if (dict[dictSize]) {
                    strcpy(dict[dictSize], wc);
                    dictSize++;
                } else {
                    // Jika malloc gagal, kita tetap melanjutkan (dict tidak bertambah)
                    printf("Peringatan: malloc gagal saat menambah kamus, mengabaikan entri baru.\n");
                }
            }
            
            // Set w menjadi karakter c saat ini
            w[0] = (char)c;
            w[1] = '\0';
        }
    }

    // Tulis kode terakhir untuk w
    int code = findString(dict, dictSize, w);
    uint16_t outcode = (uint16_t)code;
    fwrite(&outcode, sizeof(uint16_t), 1, out);

    // Bebaskan semua memori kamus
    for (int i = 0; i < dictSize; i++)
        free(dict[i]);

    fclose(in);
    fclose(out);

    printf("KOMPRESI SELESAI → %s\n", output);
}