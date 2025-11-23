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

    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {
        dict[i] = malloc(2);
        dict[i][0] = (char)i;
        dict[i][1] = '\0';
        dictSize++;
    }

    uint16_t prevCode;
    if (fread(&prevCode, sizeof(uint16_t), 1, in) != 1) {
        fclose(in);
        fclose(out);
        return;
    }

    fwrite(dict[prevCode], 1, strlen(dict[prevCode]), out);

    char prevStr[1024];
    strcpy(prevStr, dict[prevCode]);

    uint16_t newCode;

    while (fread(&newCode, sizeof(uint16_t), 1, in) == 1) {
        char entry[1024];

        if (newCode < dictSize) {
            strcpy(entry, dict[newCode]);
        } else {
            snprintf(entry, sizeof(entry), "%s%c", prevStr, prevStr[0]);
        }

        fwrite(entry, 1, strlen(entry), out);

        if (dictSize < MAX_DICT_SIZE) {
            char *newEntry = malloc(strlen(prevStr) + 2);
            snprintf(newEntry, strlen(prevStr) + 2, "%s%c", prevStr, entry[0]);
            dict[dictSize++] = newEntry;
        }

        strcpy(prevStr, entry);
    }

    for (int i = 0; i < dictSize; i++)
        free(dict[i]);

    fclose(in);
    fclose(out);

    printf("DEKOMPRESI SELESAI → %s\n", output);
}
