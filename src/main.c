#include <stdio.h>
#include "lzw.h"

int main() {
    int pilih;
    char in[256], out[256];

    do {
        printf("\n===== MENU LZW =====\n");
        printf("1. Kompres\n");
        printf("2. Dekompres\n");
        printf("3. Keluar\n");
        printf("Pilih: ");
        scanf("%d", &pilih);

        if (pilih == 1) {
            printf("File input (.txt): ");
            scanf("%s", in);
            printf("File output (.lzw): ");
            scanf("%s", out);
            compressFile(in, out);
        }
        else if (pilih == 2) {
            printf("File input (.lzw): ");
            scanf("%s", in);
            printf("File output (.txt): ");
            scanf("%s", out);
            decompressFile(in, out);
        }
    } while (pilih != 3);

    return 0;
}
