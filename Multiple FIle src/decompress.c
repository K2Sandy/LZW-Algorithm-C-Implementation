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
    char *prevStr = strdup(dict[prevCode]); // Alokasi dinamis untuk prevStr
    fwrite(prevStr, 1, strlen(prevStr), out);

    uint16_t newCode;
    
    // Alokasi dinamis untuk entry (berpotensi tumbuh besar)
    char *entry = NULL;

    while (fread(&newCode, sizeof(uint16_t), 1, in) == 1) {
        
        char *currentEntry; // Pointer sementara
        int isKSK = 0;      // Flag untuk kasus K-S-K

        if (newCode < dictSize) {
            // Kasus normal
            currentEntry = dict[newCode];
        } else if (newCode == dictSize) {
            // Kasus K-S-K (Kode baru yang baru saja dibuat)
            // entry = prevStr + prevStr[0]
            isKSK = 1;
            
            // Alokasi memori untuk K-S-K (panjang prevStr + 2 untuk karakter pertama + \0)
            entry = realloc(entry, strlen(prevStr) + 2);
            if (!entry) { /* Penanganan error */ break; } 
            
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
            // C = karakter pertama dari currentEntry
            
            // Alokasi memori untuk entri kamus baru (panjang prevStr + 2)
            char *newEntry = malloc(strlen(prevStr) + 2);
            if (!newEntry) { /* Penanganan error */ break; }

            // Buat P + C
            strcpy(newEntry, prevStr);
            newEntry[strlen(prevStr)] = currentEntry[0];
            newEntry[strlen(prevStr) + 1] = '\0';

            dict[dictSize++] = newEntry;
        }
        
        // Perbarui prevStr untuk iterasi berikutnya
        // Bebaskan prevStr lama, dan alokasikan/salin prevStr baru
        free(prevStr); 
        prevStr = strdup(currentEntry);
        
        // Jika K-S-K, kita harus membebaskan memori 'entry' karena strdup sudah menyalinnya
        if (isKSK) {
            // Jika KSK, entry adalah buffer sementara yang dialokasikan ulang/baru
            // Biarkan 'entry' pointer tetap ada untuk realloc di iterasi berikutnya
            // Namun, kita tidak perlu memanggil free(entry) di sini karena kita akan realloc/reuse.
            // Biarkan free(entry) di akhir jika entry bukan NULL.
        }
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