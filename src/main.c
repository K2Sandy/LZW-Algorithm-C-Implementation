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
        buffer[0] = '\0'; // Set to empty string on error
        return;
    }
    
    // Remove the newline or carriage return character from the end
    buffer[strcspn(buffer, "\r\n")] = 0;
}

int main() {
    int pilih;
    char in[MAX_PATH_LEN], out[MAX_PATH_LEN];

    do {
        printf("\n===== MENU LZW =====\n");
        printf("1. Kompres\n");
        printf("2. Dekompres\n");
        printf("3. Keluar\n");
        printf("Pilih: ");
        
        // Baca pilihan (masih bisa pakai scanf untuk integer)
        if (scanf("%d", &pilih) != 1) {
            printf("Input tidak valid. Silakan coba lagi.\n");
            clear_input_buffer(); // Clear buffer after failed scanf
            pilih = 0; 
            continue;
        }
        
        // !!! PENTING: Clear buffer setelah sukses membaca integer !!!
        clear_input_buffer(); 

        if (pilih == 1) {
            printf("File input (.txt): ");
            get_path_input(in, sizeof(in));
            printf("File output (.lzw): ");
            get_path_input(out, sizeof(out));
            
            if (strlen(in) > 0 && strlen(out) > 0) {
                // If the path is successfully read, try to compress
                compressFile(in, out);
            } else {
                printf("Jalur file tidak boleh kosong.\n");
            }

        }
        else if (pilih == 2) {
            printf("File input (.lzw): ");
            get_path_input(in, sizeof(in));
            printf("File output (.txt): ");
            get_path_input(out, sizeof(out));
            
            if (strlen(in) > 0 && strlen(out) > 0) {
                // If the path is successfully read, try to decompress
                decompressFile(in, out);
            } else {
                printf("Jalur file tidak boleh kosong.\n");
            }
        }
    } while (pilih != 3);

    return 0;
}