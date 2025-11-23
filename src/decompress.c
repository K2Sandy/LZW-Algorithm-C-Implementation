#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzw.h"

char *searchCode(DictItem dict[], int dictSize, int code) {
    for (int i = 0; i <dictSize; i++)
        if (dict[i].code == code)
            return dict[i].value;
    return NULL;
}

void decompressFile(char *inFile, char *outFile) {
    FILE *in = fopen(inFile, "rb");
    if (!in) { printf("File input tidak ditemukan!\n"); return; }
    FILE *out = fopen(outFile, "w");

    DictItem dict[MAX_DICT_SIZE];
    int dictSize = 256;

    for (int i = 0; i < 256; i++) {
        dict[i].code = i;
        dict[i].value[0] = (char)i;
        dict[i].value[1] = '\0';
    }

    int prevCode, newCode;
    fread(&prevCode, sizeof(int), 1, in);
    fprintf(out, "%s", searchCode(dict, dictSize, prevCode));

    char prevStr[MAX_WORD], entry[MAX_WORD];
    strcpy(prevStr, searchCode(dict, dictSize, prevCode));

    while (fread(&newCode, sizeof(int), 1, in) == 1) {
        char *str = searchCode(dict, dictSize, newCode);

        if (str != NULL) {
            fprintf(out, "%s", str);
            sprintf(entry, "%s%c", prevStr, str[0]);
        } else {
            sprintf(entry, "%s%c", prevStr, prevStr[0]);
            fprintf(out, "%s", entry);
        }

        if (dictSize < MAX_DICT_SIZE) {
            dict[dictSize].code = dictSize;
            strcpy(dict[dictSize].value, entry);
            dictSize++;
        }

        strcpy(prevStr, searchCode(dict, dictSize, newCode));
    }

    fclose(in);
    fclose(out);
    printf("DEKOMPRESI SELESAI → file output: %s\n", outFile);
}
