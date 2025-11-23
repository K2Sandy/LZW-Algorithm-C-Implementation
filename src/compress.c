#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lzw.h"

static int findString(char **dict, int size, const char *str) {
    for (int i = 0; i < size; i++) {
        if (dict[i] && strcmp(dict[i], str) == 0)
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

    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {
        dict[i] = malloc(2);
        dict[i][0] = (char)i;
        dict[i][1] = '\0';
        dictSize++;
    }

    int c = fgetc(in);
    if (c == EOF) {
        fclose(in);
        fclose(out);
        return;
    }

    char w[1024] = {0};
    w[0] = (char)c;
    w[1] = '\0';

    while ((c = fgetc(in)) != EOF) {
        char wc[1024];
        snprintf(wc, sizeof(wc), "%s%c", w, (char)c);

        if (findString(dict, dictSize, wc) != -1) {
            strcpy(w, wc);
        } else {
            int code = findString(dict, dictSize, w);
            uint16_t outcode = (uint16_t)code;
            fwrite(&outcode, sizeof(uint16_t), 1, out);

            if (dictSize < MAX_DICT_SIZE) {
                dict[dictSize] = malloc(strlen(wc) + 1);
                strcpy(dict[dictSize], wc);
                dictSize++;
            }
            w[0] = (char)c;
            w[1] = '\0';
        }
    }

    int code = findString(dict, dictSize, w);
    uint16_t outcode = (uint16_t)code;
    fwrite(&outcode, sizeof(uint16_t), 1, out);

    for (int i = 0; i < dictSize; i++)
        free(dict[i]);

    fclose(in);
    fclose(out);

    printf("KOMPRESI SELESAI → %s\n", output);
}
