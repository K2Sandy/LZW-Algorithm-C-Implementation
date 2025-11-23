#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzw.h"

int searchDictionary(DictItem dict[], int dictSize, char *value) {
    for (int i = 0; i < dictSize; i++)
        if (strcmp(dict[i].value, value) == 0)
            return dict[i].code;
    return -1;
}

void compressFile(char *inFile, char *outFile) {
    FILE *in = fopen(inFile, "r");
    if (!in) { printf("File input tidak ditemukan!\n"); return; }
    FILE *out = fopen(outFile, "wb");

    DictItem dict[MAX_DICT_SIZE];
    int dictSize = 256;

    for (int i = 0; i < 256; i++) {
        dict[i].code = i;
        dict[i].value[0] = (char)i;
        dict[i].value[1] = '\0';
    }

    char w[MAX_WORD], wc[MAX_WORD];
    int c;
    w[0] = '\0';

    while ((c = fgetc(in)) != EOF) {
        sprintf(wc, "%s%c", w, c);
        if (searchDictionary(dict, dictSize, wc) != -1) {
            strcpy(w, wc);
        } else {
            int code = searchDictionary(dict, dictSize, w);
            fwrite(&code, sizeof(int), 1, out);

            if (dictSize < MAX_DICT_SIZE) {
                dict[dictSize].code = dictSize;
                strcpy(dict[dictSize].value, wc);
                dictSize++;
            }

            w[0] = c;
            w[1] = '\0';
        }
    }

    if (strlen(w) > 0) {
        int code = searchDictionary(dict, dictSize, w);
        fwrite(&code, sizeof(int), 1, out);
    }

    fclose(in);
    fclose(out);
    printf("KOMPRESI SELESAI → file output: %s\n", outFile);
}
