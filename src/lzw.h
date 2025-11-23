#ifndef LZW_H
#define LZW_H

#define MAX_DICT_SIZE 4096
#define MAX_WORD 50

typedef struct {
    int code;
    char value[MAX_WORD];
} DictItem;

void compressFile(char *inFile, char *outFile);
void decompressFile(char *inFile, char *outFile);

#endif
