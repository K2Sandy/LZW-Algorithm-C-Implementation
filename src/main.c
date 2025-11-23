#include <stdio.h>
#include <string.h>
#include "lzw.h"

void inputPath(char *buffer) {
    // buang newline dari input sebelumnya
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    fgets(buffer, 260, stdin);

    // hapus newline di akhir string
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
}

int main() {
    int pilih;
    char in[260], out[260];

    do {
        printf("\n===== MENU LZW =====\n");
        printf("1. Kompres\n");
        printf("2. Dekompres\n");
        printf("3. Keluar\n");
        printf("Pilih: ");
        scanf("%d", &pilih);

        switch (pilih) {
        case 1:
            printf("File input (.txt): ");
            inputPath(in);

            printf("File output (.lzw): ");
            inputPath(out);

            compressFile(in, out);
            break;

        case 2:
            printf("File input (.lzw): ");
            inputPath(in);

            printf("File output (.txt): ");
            inputPath(out);

            decompressFile(in, out);
            break;

        case 3:
            printf("Program selesai.\n");
            break;

        default:
            printf("Pilihan tidak valid!\n");
        }

    } while (pilih != 3);

    return 0;
}
