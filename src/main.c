#include <stdio.h>
#include <string.h> 
#include "lzw.h"

#define MAX_PATH_LEN 256

// Helper function to clear the input buffer (stdin)
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Helper function to read a path, handling spaces and newlines
void get_path_input(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0'; 
        return;
    }
    
    // Remove the newline or carriage return character from the end
    buffer[strcspn(buffer, "\r\n")] = 0;
}

// FUNGSI BARU: Menambahkan ekstensi jika belum ada
void add_extension(char *filename, const char *ext, size_t max_len) {
    size_t file_len = strlen(filename);
    size_t ext_len = strlen(ext);

    // Cek apakah ekstensi sudah ada
    if (file_len >= ext_len && strcmp(filename + file_len - ext_len, ext) == 0) {
        return; // Ekstensi sudah ada, tidak perlu ditambahkan
    }

    // Cek apakah ada cukup ruang di buffer
    if (file_len + ext_len + 1 <= max_len) {
        // Tambahkan ekstensi
        strcat(filename, ext);
    } else {
        // Penanganan error: buffer penuh
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
            printf("File input (.txt)   : ");
            get_path_input(in, sizeof(in));
            printf("File output (.lzw)  : ");
            get_path_input(out, sizeof(out));
            
            if (strlen(in) > 0 && strlen(out) > 0) {
                // Tambahkan .lzw ke output file
                add_extension(out, ".lzw", sizeof(out));
                compressFile(in, out);
            } else {
                printf("Jalur file tidak boleh kosong.\n");
            }

        }
        else if (pilih == 2) {
            printf("File input (.lzw)   : ");
            get_path_input(in, sizeof(in));
            printf("File output (.txt)  : ");
            get_path_input(out, sizeof(out));
            
            if (strlen(in) > 0 && strlen(out) > 0) {
                // Tambahkan .txt ke output file
                add_extension(out, ".txt", sizeof(out));
                decompressFile(in, out);
            } else {
                printf("Jalur file tidak boleh kosong.\n");
            }
        }
    } while (pilih != 3);

    return 0;
}