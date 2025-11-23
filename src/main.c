#include <stdio.h>
#include <stdlib.h>
#include "lzw.h"

int main() {
    int pilihan;
    char fileIn[100], fileOut[100];

    do {
        printf("\n==================== MENU LZW ====================\n");
        printf("1. Kompres File\n");
        printf("2. Dekompres File\n");
        printf("3. Keluar\n");
        printf("==================================================\n");
        printf("Pilih menu: ");
        scanf("%d", &pilihan);

        switch (pilihan) {
        case 1:
            printf("Masukkan nama file input (.txt): ");
            scanf("%s", fileIn);
            printf("Masukkan nama file output (.lzw): ");
            scanf("%s", fileOut);
            compressFile(fileIn, fileOut);
            break;

        case 2:
            printf("Masukkan nama file input (.lzw): ");
            scanf("%s", fileIn);
            printf("Masukkan nama file output (.txt): ");
            scanf("%s", fileOut);
            decompressFile(fileIn, fileOut);
            break;

        case 3:
            printf("Program selesai.\n");
            break;

        default:
            printf("Pilihan tidak valid!\n");
        }
    } while (pilihan != 3);

    return 0;
}
