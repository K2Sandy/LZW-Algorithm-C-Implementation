#include <stdio.h>
#include <string.h>
#include "lzw.h"
#include "dialog.h"

#define MAX_PATH_LEN  MAX_PATH

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void add_extension(char *filename, const char *ext, size_t max_len) {
    size_t file_len = strlen(filename);
    size_t ext_len = strlen(ext);
    if (file_len >= ext_len && strcmp(filename + file_len - ext_len, ext) == 0) {
        return;
    }
    if (file_len + ext_len + 1 <= max_len) {
        strcat(filename, ext);
    } else {
        printf("Peringatan: Nama file terlalu panjang, ekstensi tidak ditambahkan.\n");
    }
}

int main() {
    int pilih;
    char in[MAX_PATH_LEN], out[MAX_PATH_LEN];

    do {
        printf("\n=============================================");
        printf("\n   Kompresi File Text Dengan Algoritma LZW   \n");
        printf("=============================================\n");
        printf("1. Kompres\n");
        printf("2. Dekompres\n");
        printf("3. Keluar\n");
        printf("=============================================\n");
        printf("Pilih: ");

        if (scanf("%d", &pilih) != 1) {
            printf("Input tidak valid. Silakan coba lagi.\n");
            clear_input_buffer();
            pilih = 0;
            continue;
        }
        clear_input_buffer();

        if (pilih == 1) {
            printf("Pilih file input (.txt)...\n");
            if (!openFileDialog(in, sizeof(in), "Text Files\0*.txt\0All Files\0*.*\0")) {
                printf("Tidak ada file yang dipilih (dibatalkan).\n");
                continue;
            }

            printf("Pilih lokasi dan nama file output (.lzw)...\n");
            strncpy(out, "output.lzw", sizeof(out)-1);
            out[sizeof(out)-1] = '\0';
            if (!saveFileDialog(out, sizeof(out), "LZW Files\0*.lzw\0All Files\0*.*\0", "lzw")) {
                printf("Penyimpanan dibatalkan.\n");
                continue;
            }

            // ensure extension .lzw
            size_t file_len = strlen(out);
            if (file_len < 4 || strcmp(out + file_len - 4, ".lzw") != 0) {
                if (file_len + 4 < sizeof(out)) strcat(out, ".lzw");
            }

            compressFile(in, out);
        }
        else if (pilih == 2) {
            printf("Pilih file input (.lzw)...\n");
            if (!openFileDialog(in, sizeof(in), "LZW Files\0*.lzw\0All Files\0*.*\0")) {
                printf("Tidak ada file yang dipilih (dibatalkan).\n");
                continue;
            }

            printf("Pilih lokasi dan nama file output (.txt)...\n");
            strncpy(out, "output.txt", sizeof(out)-1);
            out[sizeof(out)-1] = '\0';
            if (!saveFileDialog(out, sizeof(out), "Text Files\0*.txt\0All Files\0*.*\0", "txt")) {
                printf("Penyimpanan dibatalkan.\n");
                continue;
            }

            // ensure extension .txt
            size_t file_len = strlen(out);
            if (file_len < 4 || strcmp(out + file_len - 4, ".txt") != 0) {
                if (file_len + 4 < sizeof(out)) strcat(out, ".txt");
            }

            decompressFile(in, out);
        }
    } while (pilih != 3);

    return 0;
}