#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lzw.h"

void decompressFile(const char *input, const char *output) {
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan!\n"); return; }

    FILE *out = fopen(output, "wb");
    if (!out) { printf("Gagal membuka file output!\n"); fclose(in); return; }

    char *dict[MAX_DICT_SIZE] = {0};
    int dictSize = 0;

    // Inisialisasi Kamus
    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {
        dict[i] = malloc(2);
        if (!dict[i]) {
            printf("Error: malloc gagal saat inisialisasi kamus\n");
            for (int j = 0; j < i; j++) free(dict[j]);
            fclose(in); fclose(out);
            return;
        }
        dict[i][0] = (char)i;
        dict[i][1] = '\0';
        dictSize++;
    }

    uint16_t prevCode;
    if (fread(&prevCode, sizeof(uint16_t), 1, in) != 1) {
        // Bebaskan kamus sebelum keluar
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in);
        fclose(out);
        return;
    }
    
    // Penanganan: pastikan prevCode valid
    if (prevCode >= dictSize) {
        printf("Error: Kode pertama tidak valid!\n");
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in);
        fclose(out);
        return;
    }

    // Tulis string pertama (W)
    char *prevStr = _strdup(dict[prevCode]); // gunakan _strdup untuk kompatibilitas MSVC
    if (!prevStr) {
        printf("Error: strdup gagal saat alokasi prevStr\n");
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in); fclose(out);
        return;
    }
    fwrite(prevStr, 1, strlen(prevStr), out);

    uint16_t newCode;
    
    // Buffer sementara untuk kasus K-S-K
    char *entry = NULL;
    size_t entry_buf_len = 0;

    while (fread(&newCode, sizeof(uint16_t), 1, in) == 1) {
        
        char *currentEntry = NULL; // Pointer sementara
        int isKSK = 0;      // Flag untuk kasus K-S-K

        if (newCode < (uint16_t)dictSize) {
            // Kasus normal
            currentEntry = dict[newCode];
        } else if (newCode == (uint16_t)dictSize) {
            // Kasus K-S-K (Kode baru yang baru saja dibuat)
            isKSK = 1;
            size_t need = strlen(prevStr) + 2;
            if (entry_buf_len < need) {
                char *tmp = realloc(entry, need);
                if (!tmp) {
                    printf("Error: realloc gagal saat membuat entry K-S-K\n");
                    break;
                }
                entry = tmp;
                entry_buf_len = need;
            }
            strcpy(entry, prevStr);
            entry[strlen(prevStr)] = prevStr[0];
            entry[strlen(prevStr) + 1] = '\0';
            currentEntry = entry;
        } else {
            // Error: Kode tidak valid/di luar kamus yang diharapkan
            printf("Error: Kode %u di luar batas kamus (%d)!\n", newCode, dictSize);
            break;
        }

        // Tulis string entry
        fwrite(currentEntry, 1, strlen(currentEntry), out);

        // Tambahkan string baru (P + C) ke kamus
        if (dictSize < MAX_DICT_SIZE) {
            // Alokasi memori untuk entri kamus baru (panjang prevStr + 2)
            char *newEntry = malloc(strlen(prevStr) + 2);
            if (!newEntry) {
                printf("Error: malloc gagal saat menambah entri kamus\n");
                break;
            }

            // Buat P + C
            strcpy(newEntry, prevStr);
            newEntry[strlen(prevStr)] = currentEntry[0];
            newEntry[strlen(prevStr) + 1] = '\0';

            dict[dictSize++] = newEntry;
        }

        // Perbarui prevStr untuk iterasi berikutnya
        free(prevStr); 
        prevStr = _strdup(currentEntry);
        if (!prevStr) {
            printf("Error: strdup gagal saat update prevStr\n");
            break;
        }

        // note: kita tidak free(entry) di sini karena kita reuse buffer entry dengan realloc
    }

    // Bebaskan semua memori kamus
    for (int i = 0; i < dictSize; i++)
        free(dict[i]);
    
    // Bebaskan prevStr dan entry yang tersisa
    if (prevStr) free(prevStr);
    if (entry) free(entry);

    fclose(in);
    fclose(out);

    printf("DEKOMPRESI SELESAI → %s\n", output);
}