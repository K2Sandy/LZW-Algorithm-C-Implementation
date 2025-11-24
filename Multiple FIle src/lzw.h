#ifndef LZW_H
#define LZW_H

#include <stdint.h>
#include <windows.h>

#define MAX_DICT_SIZE 4096     // 12-bit dictionary
#define INITIAL_DICT_SIZE 256  // ASCII chars

void compressFile(const char *input, const char *output);
void decompressFile(const char *input, const char *output);

int openFileDialog(char *outPath, DWORD outSize, const char *filter);
int saveFileDialog(char *outPath, DWORD outSize, const char *filter, const char *defExt);

#endif
