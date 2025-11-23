#ifndef LZW_H
#define LZW_H

#include <stdint.h>

#define MAX_DICT_SIZE 4096     // 12-bit dictionary
#define INITIAL_DICT_SIZE 256  // ASCII chars

void compressFile(const char *input, const char *output);
void decompressFile(const char *input, const char *output);

#endif
